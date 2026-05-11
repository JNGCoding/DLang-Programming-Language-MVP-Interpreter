#ifndef LANGFILES_UTILITY_DEFAULT_STREAMS_HPP__
#define LANGFILES_UTILITY_DEFAULT_STREAMS_HPP__

#include "../io/input_stream.hpp"
#include "../io/output_stream.hpp"

#include <iostream>
#include <string.h>
#include <chrono>

// OUTPUT STREAM FUNCTIONS
class cout_output_stream_t : public output_stream_t {
public:
    cout_output_stream_t(): output_stream_t() {}

    int16_t write(uint8_t b) override {
        std::cout << (char) b;
        return 0;
    }
    
    int16_t print(const char* str) override {
        std::cout << str;
        return 0;
    }
    
    int16_t println(const char* str) override {
        std::cout << str << '\n';
        return 0;
    }
    
    int16_t flush() override {
        std::cout << std::flush;
        return 0;
    }
};

// INPUT STREAM FUNCTIONS
class cin_input_stream_t : public input_stream_t {
private:
    std::chrono::_V2::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    uint32_t millis() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        return static_cast<uint32_t>(elapsed);
    }
public:
    cin_input_stream_t() : input_stream_t() {
        // Don't skip white-spaces
        std::cin >> std::noskipws;
    }

    int16_t available() override {
        return std::cin.rdbuf()->in_avail();
    }

    int16_t read() override {
        char c;
        if (std::cin.get(c)) {
            return static_cast<uint8_t>(c);
        }
        return -1;
    }

    int16_t read(uint16_t wait) {
        uint32_t st = this->millis();
        while ((this->millis() - st) <= wait) {
            if (this->available() > 0) return this->read();
        }

        return -1;
    }

    int16_t readBytesUntil(uint8_t* b, uint32_t size) override {
        std::cin.read(reinterpret_cast<char*>(b), size);
        return static_cast<int>(std::cin.gcount());
    }

    int16_t readString(char* str, uint32_t str_size) override {
        if (str_size == 0) return 0;
        std::cin.getline(str, str_size);
        return static_cast<int>(strlen(str));
    }

    int16_t readStringUntil(char* str, uint32_t max_str_size, char delim) override {
        if (max_str_size == 0) return 0;
        std::cin.getline(str, max_str_size, delim);
        return static_cast<int>(strlen(str));
    }

    int16_t clear() override {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return 0;
    }
};

#endif