#ifndef STACK_MEASUREMENT_HPP
#define STACK_MEASUREMENT_HPP

#include "connection.hpp"

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

class StackMeasurementLine {
    private:
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

        uint32_t heap = 0;
        uint32_t heap_gap = 0;
        uint32_t stack = 0xFFFFFFFF;

    public:
        uint8_t id = 0;
        StackMeasurementLine() {}

        void update() {
            //packIf(&data, &__data_start, &__data_end);
            //packIf(&bss, &__bss_start, &__bss_end);
            heap = packIf(heap, __malloc_heap_start, __malloc_heap_end);
            heap_gap = packIf(heap_gap, __brkval, __malloc_margin);
            stack = packIfLessThan(stack, SP, RAMEND);
        }

        void fill(Response &response) {
            response.payload.stack_measurement.id = id;
            response.payload.stack_measurement.heap = heap;
            response.payload.stack_measurement.heap_gap = heap_gap;
            response.payload.stack_measurement.stack = stack;
        }
};

template<uint8_t COUNT>
class StackMeasurer {
    private:
        StackMeasurementLine measurements[COUNT];

    public:
        StackMeasurer() {
            for (uint8_t i = 0; i < COUNT; i += 1) {
                measurements[i].id = COUNT;
            }
        }

        void update(const int id) {
            if (measurements[id - 1].id == COUNT) {
                measurements[id - 1].id = id - 1;
            }
            measurements[id - 1].update();
        }

        template <size_t BufferSize, unsigned long BAUD_RATE, uint8_t STATUS_LED>
        void send(Connection<BufferSize, BAUD_RATE, STATUS_LED> &connection) {
            Response response;
            response.which_payload = Response_stack_measurement_tag;
            response.payload.stack_measurement.data = pack(&__data_start, &__data_end);
            response.payload.stack_measurement.bss = pack(&__bss_start, &__bss_end);
            for (int i = 0; i < COUNT; i += 1) {
                if (measurements[i].id != COUNT) {
                    measurements[i].fill(response);
                    connection.send(response);
                }
            }
        } };

#endif /* STACK_MEASUREMENT_HPP */
