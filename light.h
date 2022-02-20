#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <FastLED.h>

enum ahds { attack = 'a', hold = 'h', decay = 'd', sustain = 's' };

class Light {
public:
    boolean enabled = false;
    uint8_t id;
    uint16_t start;
    uint16_t end;
    CRGB start_color_A;
    CRGB start_color_B;
    CRGB end_color_A;
    CRGB end_color_B;

    // Animation Parameters
    uint16_t attack;
    uint16_t hold;
    uint16_t decay;
    uint16_t sustain;

    CRGB* leds = nullptr;

    void reset();

private:

    uint32_t previous_time = 0;
    fract16 t = 0;
    ahds state = ahds::attack;

    inline fract16 fraction(uint16_t start, uint16_t end, uint16_t index)
    {
        return (fract16)((((uint32_t)(index - start)) * UINT16_MAX) / ((uint32_t)(end - start)));
    }

    void set_leds(fract16 t);
public:
    void update(const uint32_t& dt);
    void log();
};

#endif
