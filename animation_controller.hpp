#ifndef ANIMATION_CONTROLLER_HPP
#define ANIMATION_CONTROLLER_HPP

#include <FastLED.h>
#include "light.h"
#include "messages.pb.h"

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
class AnimationController {
private:
    Light lights[MAX_LIGHTS];
    SetLight staged_lights[MAX_LIGHTS];

    uint16_t frames = 0;
    uint16_t fps = 0;
    unsigned long last_fps_timestamp = 0;

    CRGB leds[MAX_LEDS];
    uint64_t commit_update_time = 0;
    uint64_t last_commit_time = 0;
    boolean did_update = false;

    void move_buffers();
    void update_light(SetLight set_light);

public:
    const uint16_t number_of_lights = MAX_LIGHTS;
    const uint16_t leds_count = MAX_LEDS;

    CRGB* get_leds() const {
        return leds;
    }

    AnimationController() {
        for (uint8_t i = 0; i < MAX_LIGHTS; i += 1) {
            lights[i].leds = leds;
            lights[i].id = i;
        }

        for (uint8_t i = 0; i < MAX_LIGHTS; i += 1) {
            staged_lights[i].id = MAX_LIGHTS;
            staged_lights[i].id = MAX_LIGHTS;
        }
    }

    void update_frames_per_second_statistic(unsigned long current_time);

    inline boolean is_valid_light(uint8_t id)
    {
        return id < MAX_LIGHTS;
    }

    boolean update_command(SetLight &set_light);

    Light& operator[](uint8_t index);

    void log_state();

    void update(const uint32_t& dt);

    void set_commit_time(const uint64_t timestamp);
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
Light& AnimationController<MAX_LIGHTS, MAX_LEDS>::operator[](uint8_t index)
{
    return lights[index];
}

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
void AnimationController<MAX_LIGHTS, MAX_LEDS>::update(const uint32_t& dt)
{
    if (did_update && millis() - last_commit_time >= commit_update_time) {
        // move staged buffers into the display buffers
        move_buffers();
        did_update = false;
        last_commit_time = millis();
        commit_update_time = 0;
    }

    for (int i = 0; i < number_of_lights; i += 1) {
        lights[i].update(dt);
    }
}

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
boolean AnimationController<MAX_LIGHTS, MAX_LEDS>::update_command(SetLight &set_light)
{
    if (!is_valid_light(set_light.id)) {
        return false;
    }

    uint16_t start = set_light.range >> 16;
    uint16_t end = set_light.range & 0xFFFF;
    if (start > end) {
        start = set_light.range & 0xFFFF;
        end = set_light.range >> 16;
    }

    if (start >= this->leds_count || end > this->leds_count) {
        return false;
    }

    staged_lights[set_light.id] = set_light;

    return true;
}

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
void AnimationController<MAX_LIGHTS, MAX_LEDS>::set_commit_time(uint64_t timestamp)
{
    commit_update_time = timestamp;
    did_update = true;
}


template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
void AnimationController<MAX_LIGHTS, MAX_LEDS>::update_light(SetLight set_light)
{
    uint16_t start = set_light.range >> 16;
    uint16_t end = set_light.range & 0xFFFF;
    if (start > end) {
        start = set_light.range & 0xFFFF;
        end = set_light.range >> 16;
    }

    Light &light = this->operator[](set_light.id);
    light.start = start;
    light.end = end;

    light.enabled = light.start != light.end;

    light.attack = (set_light.ahds >> 24) & 0xFFFF;
    light.hold = (set_light.ahds >> 16) & 0xFFFF;
    light.decay = (set_light.ahds >> 8) & 0xFFFF;
    light.sustain = (set_light.ahds >> 0) & 0xFFFF;

    light.start_color_A.setColorCode(set_light.start_color);
    light.start_color_B.setColorCode(set_light.start_color_alt);
    light.end_color_A.setColorCode(set_light.end_color);
    light.end_color_B.setColorCode(set_light.end_color_alt);

    light.reset();
}

template <uint8_t MAX_LIGHTS, uint16_t MAX_LEDS>
void AnimationController<MAX_LIGHTS, MAX_LEDS>::move_buffers()
{
    for (size_t i = 0; i < MAX_LIGHTS; i += 1) {
        if (staged_lights[i].id < MAX_LIGHTS) {
            update_light(staged_lights[i]);
            staged_lights[i].id = MAX_LIGHTS;
        }
    }
}


#endif
