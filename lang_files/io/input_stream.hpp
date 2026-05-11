#ifndef LANGFILES_IO_INPUT_STREAM_HPP__
#define LANGFILES_IO_INPUT_STREAM_HPP__

#include <stdint.h>

class input_stream_t {
public:
    input_stream_t() {}
    virtual int16_t available() = 0;
    virtual int16_t read() = 0;
    virtual int16_t readBytesUntil(uint8_t* b, uint32_t size) = 0;
    virtual int16_t readString(char* str, uint32_t str_size) = 0;
    virtual int16_t readStringUntil(char* str, uint32_t max_str_size, char delim) = 0;
    virtual int16_t clear() = 0;
};

#endif