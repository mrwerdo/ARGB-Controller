#ifndef CONNECTION_H
#define CONNECTION_H

#include <proto/pb.h>
#include <proto/pb_decode.h>
#include <PacketSerial.h>
#include <AceCRC.h>
#include "messages.pb.h"

void sendStackMeasurements(int id);
void updateGreatestResponse();

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
        sendStackMeasurements(7);

        digitalWrite(STATUS_LED, HIGH);
        delay(200);
        digitalWrite(STATUS_LED, LOW);
        delay(200);
    }

    void processPacket(const uint8_t* buffer, size_t size) {
        //updateGreatestResponse();
        if (size <= 4) {
            // At least the CRC should be there, plus something else.
            error(ErrorCode::too_short, F("message"));
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
            error(ErrorCode::invalid_crc, F("received"));
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
            error(ErrorCode::no_callback_assigned, F("no"));
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
        const size_t max_encoded_message_size = 80;
        uint8_t buffer[max_encoded_message_size+4];
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, max_encoded_message_size);
        if (!pb_encode(&stream, Response_fields, &message)) {
            error(ErrorCode::protobuf_encode, F("failed"));
            return;
        }
        crc_t crc = crc_calculate(buffer, stream.bytes_written);
        uint8_t* crc_ptr = (uint8_t*)&crc;
        buffer[stream.bytes_written+0] = crc_ptr[0];
        buffer[stream.bytes_written+1] = crc_ptr[1];
        buffer[stream.bytes_written+2] = crc_ptr[2];
        buffer[stream.bytes_written+3] = crc_ptr[3];
        while (Serial.availableForWrite() < 8) {
            this->update();
        }
        Serial.write(0);
        Serial.write(0);
        Serial.write(0);
        Serial.write(0);
        while (Serial.availableForWrite() < stream.bytes_written + 4) {
            this->update();
        }
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
        sendStackMeasurements(6);
    }

    void update() {
        packetSerial.update();
        if (packetSerial.overflow()) {
            error(ErrorCode::overflow, F("buffer"));
        }
    }
};

#endif /* CONNECTION_H */
