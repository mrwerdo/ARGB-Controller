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
#define STARTUP_DELAY 3000
#define BRIGHTNESS 100
#define STATUS_LED 13
#define FRAMES_PER_SECOND 120

static AnimationController<4, MAXIMUM_NUMBER_OF_LEDS> animation_controller;
static Connection<64, BAUD_RATE, STATUS_LED> connection;

template<uint8_t PIN, uint8_t OFFSET, uint8_t SIZE>
CLEDController& addLeds() {
    return FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(animation_controller.get_leds(), OFFSET, SIZE);
}

extern unsigned int __data_start;
extern unsigned int __data_end;
extern unsigned int __bss_start;
extern unsigned int __bss_end;
extern void *__brkval;
extern char *__heap_start;
extern char *__heap_end;
extern size_t __malloc_margin;
extern unsigned int *__malloc_start;
extern unsigned int *__malloc_end;

static Response greatest_response;

// this function will return the number of bytes currently free in RAM
int freemem()
{
    int free_memory;
    if((int)__brkval == 0)
        free_memory = ((int)&free_memory) - ((int)&__bss_end);
    else
        free_memory = ((int)&free_memory) - ((int)__brkval);
    return free_memory;
}

static inline size_t stack_size()
{
    return RAMEND - SP;
}

inline uint32_t pack(uint16_t a, uint16_t b)
{
    uint32_t result = a;
    result <<= 16;
    result |= b;
    return result;
}

inline uint32_t packIf(uint32_t existing, uint16_t a, uint16_t b)
{
    uint16_t x = existing & 0xFFFF;
    uint16_t y = (existing >> 16) & 0xFFFF;
    if (x > a) {
        a = x;
    }
    if (y > b) {
        b = y;
    } 
    uint32_t result = a;
    result <<= 16;
    result |= b;
    return result;
}

inline uint32_t packIfLessThan(uint32_t existing, uint16_t a, uint16_t b)
{
    uint16_t x = existing & 0xFFFF;
    uint16_t y = (existing >> 16) & 0xFFFF;
    if (x < a) {
        a = x;
    }
    if (y < b) {
        b = y;
    } 
    uint32_t result = a;
    result <<= 16;
    result |= b;
    return result;
}

void updateGreatestResponse()
{
    greatest_response.which_payload = Response_stack_measurement_tag;
    greatest_response.payload.stack_measurement.id = 123;
    greatest_response.payload.stack_measurement.data = packIf(greatest_response.payload.stack_measurement.data, &__data_start, &__data_end);
    greatest_response.payload.stack_measurement.bss = packIf(greatest_response.payload.stack_measurement.bss, &__bss_start, &__bss_end);
    greatest_response.payload.stack_measurement.heap = packIf(greatest_response.payload.stack_measurement.heap, __malloc_heap_start, __malloc_heap_end);
    greatest_response.payload.stack_measurement.heap_gap = packIf(greatest_response.payload.stack_measurement.heap_gap,__brkval, __malloc_margin);
    greatest_response.payload.stack_measurement.stack = packIfLessThan(greatest_response.payload.stack_measurement.stack, SP, RAMEND);
}

void sendStackMeasurements(int id)
{
    // At least 28 bytes
    Response response;
    response.which_payload = Response_stack_measurement_tag;
    response.payload.stack_measurement.id = id;
    response.payload.stack_measurement.data = pack(&__data_start, &__data_end);
    response.payload.stack_measurement.bss = pack(&__bss_start, &__bss_end);
    response.payload.stack_measurement.heap = pack(__malloc_heap_start, __malloc_heap_end);
    response.payload.stack_measurement.heap_gap = pack(__brkval, __malloc_margin);
    response.payload.stack_measurement.stack = pack(SP, RAMEND);
    connection.send(response);
}

void callback(const Request &message) {
    updateGreatestResponse();
    switch (message.which_payload) {
    case Request_set_light_tag: {
            //sendStackMeasurements(3);
            SetLight set_light = message.payload.set_light;
            if (animation_controller.update_command(set_light)) {
                connection.log(LogCode::set_light, F("sucessfully"));
            } else {
                connection.error(ErrorCode::no_callback_assigned, F("invalid"));
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
    connection.error(ErrorCode::unknown_message, F("unknown"));
}

void setup() {
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);

    greatest_response.payload.stack_measurement.stack = 0xFFFFFFFF;

    // The following funciton accepts PIN, OFFSET, and SIZE.
    // Offset is the accumulation of the size parameter starting at zero (that is, OFFSET+SIZE = the
    // next OFFSET). The MAXIMUM_NUMBER_OF_LEDS should be at least the sum of the SIZE parameters.
    addLeds<9, 0, 6>().setCorrection(TypicalLEDStrip);
    addLeds<10, 6, 6>().setCorrection(TypicalLEDStrip);
    addLeds<11, 12, 8>().setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    connection.initialize();
    connection.set_callback(callback);
    sendStackMeasurements(2);

    // The board's bootloader intercepts serial messages until a timeout expires.
    // Wait for the timeout to expire before sending messages.
    delay(STARTUP_DELAY);
    while (Serial.available() > 0) {
        Serial.read();
    }
    connection.log(LogCode::ready, F("ARGB Controller Ready"));
    sendStackMeasurements(4);
    updateGreatestResponse();
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
    /*
    I'm going to guess the problem is that buffers are being overwritten during the middle of
    a write. This occurs because the write command has not finished sending the buffers?

    Another theory is that the 356 bytes are being used by the stack, which is possible. I was
    trying to prove this theory but collecting evidence is elusive. I believe the necessary
    functions for sending the measurements take up too much stack and also cannot be sent.

    The board does not continue sending stack measurements, so I think it's safe to say it cannot
    be the first theory. Another observation is that measuring the stack just before a packet is 
    processed seems to prevent any other messages from being sent.

    Okay. What is going on is as follows:
    1. The stack is being exhausted.
    2. __malloc_heap_end is set to a value near __malloc_heap_start to signal to the program memory exhaustion.
    3. The messages are being received part way through which results in out of sync programs.
    */
    connection.update();
    connection.send(greatest_response);
    //sendStackMeasurements(1);
    //controller_update();
}

