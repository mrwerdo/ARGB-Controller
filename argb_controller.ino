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
#define MAXIMUM_NUMBER_OF_LEDS 17
#define BAUD_RATE 9600
#define STARTUP_DELAY 1000
#define BRIGHTNESS 100
#define STATUS_LED 13
#define FRAMES_PER_SECOND 120

static AnimationController<4, MAXIMUM_NUMBER_OF_LEDS> animation_controller;
static Connection<64, BAUD_RATE, STATUS_LED> connection;

void processMessage(const uint8_t* buffer, size_t size);
void update_light(Light &light, const SetLight &set_light);

void setup() {
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);

    connection.initialize([](const uint8_t* buffer, size_t size) {
        // todo: modify library so that this goes away.
        connection.processPacket(buffer, size);
    });

    connection.set_callback([](const Request &message) {
        switch (message.which_payload) {
        case Request_set_light_tag: {
                SetLight set_light = message.payload.set_light;
                if (animation_controller.is_valid_light(set_light.id)) {
                    Light &light = animation_controller[set_light.id];
                    update_light(light, set_light);
                    light.enabled = true;
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

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(animation_controller.get_leds(), animation_controller.leds_count).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    // fill_rainbow(animation_controller.get_leds(), animation_controller.leds_count, 0, 7);
    Light &light = animation_controller[0];
    light.enabled = true;
    light.start = 0;
    light.end = 16;
    light.start_color_A = CRGB(255, 0, 0);
    light.start_color_B = CRGB(255, 0, 0);
    light.end_color_A = CRGB(0, 0, 255);
    light.end_color_B = CRGB(0, 0, 255);
    light.attack = 500;
    light.hold = 500;
    light.decay = 500;
    light.sustain = 500;
    light.id = 0;
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
    FastLED.delay(1000/FRAMES_PER_SECOND);
}

void loop() {
    connection.update();
    controller_update();
}

void update_light(Light &light, const SetLight &set_light) {
    light.start = set_light.range >> 16;
    light.end = set_light.range & 0xFFFF;

    light.attack = (set_light.ahds >> 48) & 0xFFFF;
    light.hold = (set_light.ahds >> 32) & 0xFFFF;
    light.decay = (set_light.ahds >> 16) & 0xFFFF;
    light.sustain = (set_light.ahds >> 8) & 0xFFFF;

    light.start_color_A.setColorCode(set_light.start_color);
    light.start_color_B.setColorCode(set_light.start_color_alt);
    light.end_color_A.setColorCode(set_light.end_color);
    light.end_color_B.setColorCode(set_light.end_color_alt);
}
