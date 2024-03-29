#include "light.h"

void Light::set_leds(fract16 t)
{
    for (int index = start; index < end; index += 1) {
        fract16 i = fraction(start, end, index);
        CRGB start_color = start_color_A.lerp16(start_color_B, i);
        CRGB end_color = end_color_A.lerp16(end_color_B, i);
        CRGB result = start_color.lerp16(end_color, t);
        leds[index] = result;
    }
}

void Light::update(const uint32_t& dt)
{
    if (!enabled) {
        return;
    }

    uint32_t current_time = previous_time + dt;

    if (current_time > attack + hold + decay + sustain) {
        current_time -= attack + hold + decay + sustain;
    }

    if (current_time < attack) {
        fract16 t = fraction(0, attack, current_time);
        set_leds(t);
        state = ahds::attack;
        this->t = t;
    } else if (current_time < (attack + hold)) {
        set_leds(UINT16_MAX);
        state = ahds::hold;
        this->t = UINT16_MAX;
    } else if (current_time < (attack + hold + decay)) {
        fract16 t = fraction(attack + hold, attack + hold + decay, current_time);
        set_leds(UINT16_MAX - t);
        state = ahds::decay;
        this->t = t;
    } else if (current_time < (attack + hold + decay + sustain)) {
        set_leds(0);
        state = ahds::sustain;
        this->t = t;
    } else {
        // Occurs when attack, hold, decay, and sustain are all zero.
        set_leds(0);
        state = ahds::attack;
        this->t = 0;
    }

    previous_time = current_time;
}

void Light::reset() {
    this->state = ahds::attack;
    this->t = 0;
    this->previous_time = 0;
}
