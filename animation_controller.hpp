#ifndef ANIMATION_CONTROLLER_HPP
#define ANIMATION_CONTROLLER_HPP

#include <FastLED.h>
#include "light.h"
#include "log.hpp"

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
class AnimationController {
private:
    Light lights[MAX_LIGHTS];
    uint8_t number_of_lights = 0;

    uint16_t frames = 0;
    uint16_t fps = 0;
    unsigned long last_fps_timestamp = 0;

    CRGB leds[MAX_LEDS];

public:
    const uint16_t leds_count = MAX_LEDS;
    CRGB* get_leds() const {
        return leds;
    }

    AnimationController() {
        for (uint8_t i = 0; i < MAX_LIGHTS; i += 1) {
            lights[i].leds = leds;
            lights[i].id = i;
        }
    }

    void update_frames_per_second_statistic(unsigned long current_time);

    inline boolean is_valid_light(uint8_t id) { return id < MAX_LIGHTS; }

    Light& operator[](uint8_t index);

    void log_state();

    void update(const uint32_t& dt);
};

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
void AnimationController<MAX_LIGHTS, MAX_LEDS>::update_frames_per_second_statistic(unsigned long current_time)
{
    frames += 1;
    // Measure average frames per second over 10 seconds.
    if (current_time - last_fps_timestamp > 10000) {
        fps = frames / 10;
        last_fps_timestamp = current_time;
        frames = 0;
    }
}

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
Light& AnimationController<MAX_LIGHTS, MAX_LEDS>::operator[](uint8_t index) { return lights[index]; }

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
void AnimationController<MAX_LIGHTS, MAX_LEDS>::log_state()
{
    using ::log;
    log("frames per second", fps, delim::newline);
    log("number of lights", number_of_lights, delim::newline);
    for (int i = 0; i < number_of_lights; i += 1) {
        lights[i].log();
    }
}

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
void AnimationController<MAX_LIGHTS, MAX_LEDS>::update(const uint32_t& dt)
{
    for (int i = 0; i < number_of_lights; i += 1) {
        lights[i].update(dt);
    }
}

#endif