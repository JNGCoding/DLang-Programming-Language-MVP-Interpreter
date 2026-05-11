#ifndef LANGFILES_IO_OUTPUT_STREAM_HPP__
#define LANGFILES_IO_OUTPUT_STREAM_HPP__

#include <stdint.h>

class output_stream_t {
public:
    output_stream_t() {}
    virtual int16_t write(uint8_t b) = 0;
    virtual int16_t print(const char* str) = 0;
    virtual int16_t println(const char* str) = 0;
    virtual int16_t flush() = 0;
};

#endif