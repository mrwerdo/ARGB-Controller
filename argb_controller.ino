#include <FastLED.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

// Odd behaviour with arduino compiler means that we need both these includes:
#include <pb.h>
#include <proto/pb.h>

#include "light.h"
#include "log.hpp"
#include "animation_controller.hpp"
#include "connection.hpp"
#include "messages.pb.h"

#define DATA_PIN 3
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define MAXIMUM_NUMBER_OF_LEDS 20
#define BAUD_RATE 9600
#define STARTUP_DELAY 1000
#define BRIGHTNESS 100
#define STATUS_LED 13
#define FRAMES_PER_SECOND 120

static AnimationController<4, MAXIMUM_NUMBER_OF_LEDS> animation_controller;
static Connection<64, BAUD_RATE, STATUS_LED> connection;

template<uint8_t PIN, uint8_t OFFSET, uint8_t SIZE>
CLEDController& addLeds() {
    return FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(animation_controller.get_leds(), OFFSET, SIZE);
}

void setup() {
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);

    // The following funciton accepts PIN, OFFSET, and SIZE.
    // Offset is the accumulation of the size parameter starting at zero (that is, OFFSET+SIZE = the
    // next OFFSET). The MAXIMUM_NUMBER_OF_LEDS should be at least the sum of the SIZE parameters.
    addLeds<9, 0, 8>().setCorrection(TypicalLEDStrip);
    addLeds<10, 8, 6>().setCorrection(TypicalLEDStrip);
    addLeds<11, 14, 6>().setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    connection.initialize([](const uint8_t* buffer, size_t size) {
        // todo: modify library so that this goes away.
        connection.processPacket(buffer, size);
    });

    connection.set_callback([](const Request &message) {
        switch (message.which_payload) {
        case Request_set_light_tag: {
                SetLight set_light = message.payload.set_light;
                if (animation_controller.update_command(set_light)) {
                    connection.log(LogCode::set_light, F("sucessfully updated light"));
                } else {
                    connection.error(ErrorCode::no_callback_assigned, F("invalid light"));
                }
                return;
            }
        case Request_current_time_request_tag: {
                Response response;
                response.payload.current_time.timestamp = millis();
                response.which_payload = Response_current_time_tag;
                connection.send(response);
                return;
            }
        }
        connection.error(ErrorCode::unknown_message, F("unknown protobuf message"));
    });

    // The board's bootloader intercepts serial messages until a timeout expires.
    // Wait for the timeout to expire before sending messages.
    delay(STARTUP_DELAY);

    connection.log(LogCode::ready, F("ARGB Controller Ready"));
}

void controller_update()
{
    static unsigned long previous_time = 0;
    unsigned long current_time = millis();
    uint16_t dt = current_time - previous_time;
    animation_controller.update((uint32_t)dt);
    previous_time = current_time;
    FastLED.show();
    animation_controller.update_frames_per_second_statistic(current_time);
}

void loop() {
    connection.update();
    controller_update();
}
