#ifndef CONNECTION_H
#define CONNECTION_H

#include <proto/pb.h>
#include <proto/pb_decode.h>
#include <PacketSerial.h>
#include <AceCRC.h>
#include "messages.pb.h"

using namespace ace_crc::crc32_nibble;

enum ErrorCode {
    overflow = 1,
    too_short = 2,
    invalid_crc = 3,
    protobuf_decode = 4,
    protobuf_encode = 5,
    no_callback_assigned = 6,
    unknown_message = 7
};

enum LogCode {
    set_light = 8,
    ready = 9
};

template <size_t BufferSize, unsigned long BAUD_RATE, uint8_t STATUS_LED>
class Connection {
private:
    PacketSerial_<COBS, 0, BufferSize> packetSerial;

    typedef void (*PacketHandlerFunction)(const Request &message);
    PacketHandlerFunction callback = nullptr;

    /* buffer should have length == size + 4 */
    void append_checksum(uint8_t* buffer, size_t size) {
        crc_t crc = crc_calculate(buffer, size);
        uint8_t* crc_ptr = (uint8_t*)&crc;
        buffer[size+0] = crc_ptr[0];
        buffer[size+1] = crc_ptr[1];
        buffer[size+2] = crc_ptr[2];
        buffer[size+3] = crc_ptr[3];
    }

public:
    void error(ErrorCode code, String message) {
        Response response;
        response.which_payload = Response_log_tag;
        response.payload.log.is_error = true;
        response.payload.log.id = code;
        for (size_t i = 0; i < min(message.length(), 64); i += 1) {
            response.payload.log.description[i] = message[i];
        }
        response.payload.log.description[min(message.length(), 63)] = 0;
        this->send(response);

        digitalWrite(STATUS_LED, HIGH);
        delay(200);
        digitalWrite(STATUS_LED, LOW);
        delay(200);
    }

    void processPacket(const uint8_t* buffer, size_t size) {
        if (size <= 4) {
            // At least the CRC should be there, plus something else.
            error(ErrorCode::too_short, F("message contained less than 5 bytes"));
            return;
        }

        // Extract checksum.
        crc_t received_crc = 0;
        uint8_t* crc_ptr = (uint8_t*)&received_crc;
        crc_ptr[0] = buffer[size-4];
        crc_ptr[1] = buffer[size-3];
        crc_ptr[2] = buffer[size-2];
        crc_ptr[3] = buffer[size-1];

        crc_t crc = crc_calculate(buffer, size-4);
        if (received_crc != crc) {
            error(ErrorCode::invalid_crc, F("received crc different to calculated crc"));
            return;
        }

        pb_istream_t stream = pb_istream_from_buffer(buffer, size - 4);
        Request message;
        if (!pb_decode(&stream, Request_fields, &message)) {
            const char* errorMsg = PB_GET_ERROR(&stream);
            error(ErrorCode::protobuf_decode, errorMsg);
            return;
        }

        if (this->callback) {
            this->callback(message);
        } else {
            error(ErrorCode::no_callback_assigned, F("no callback assigned to (any) process messages"));
        }
    }

    void set_callback(PacketHandlerFunction callback) {
        this->callback = callback;
    }

    void initialize(typename PacketSerial_<COBS, 0, BufferSize>::PacketHandlerFunction handler) {
        packetSerial.begin(BAUD_RATE);
        // You need to pass a reference to this class's method when calling the method.
        // auto callback = &[this](const uint8_t* buffer, size_t size) {
        //     this->processPacket(buffer, size);
        // };
        packetSerial.setPacketHandler(handler);
    }

    void send(Response &message) {
        const size_t size = 80;
        uint8_t buffer[size+4];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, size);
        if (!pb_encode(&stream, Response_fields, &message)) {
            error(ErrorCode::protobuf_encode, F("failed to encode protobuf message"));
            return;
        }
        append_checksum(buffer, stream.bytes_written);
        packetSerial.send(buffer, stream.bytes_written + 4);
    }

    void log(LogCode code, String msg) {
        Response response;
        response.which_payload = Response_log_tag;
        response.payload.log.id = code;
        response.payload.log.is_error = false;
        for (size_t i = 0; i < min(msg.length(), 64); i += 1) {
            response.payload.log.description[i] = msg[i];
        }
        response.payload.log.description[min(msg.length(), 63)] = 0;
        this->send(response);
    }

    void log(LogCode code, const char *msg) {
        Response response;
        response.which_payload = Response_log_tag;
        response.payload.log.id = code;
        response.payload.log.is_error = false;
        for (size_t i = 0; i < 64; i += 1) {
            response.payload.log.description[i] = msg[i];
            if (msg[i] == 0) {
                break;
            }
        }
        this->send(response);
    }

    void update() {
        packetSerial.update();
        if (packetSerial.overflow()) {
            error(ErrorCode::overflow, F("buffer overflow detected"));
        }
    }
};

#endif /* CONNECTION_H */