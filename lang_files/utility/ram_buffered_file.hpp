#ifndef LANGFILES_UTILITY_RAM_BUFFERED_FILE_HPP__
#define LANGFILES_UTILITY_RAM_BUFFERED_FILE_HPP__

#include "../io/code_buffer.hpp"

#include <string.h>
#include <stdlib.h>

class buffered_file_t : public codefile_t {
private:
    char* data;
    bool copied = false;
    size_t filePointer = 0;
    size_t dataSize;
public:
    buffered_file_t() : codefile_t() {}

    buffered_file_t(char* d) : codefile_t() {
        this->dataSize = strlen(d);
        this->data = d;
    }

    buffered_file_t(const char* d) : codefile_t() {
        this->dataSize = strlen(d);
        this->copied = true;
        this->data = strdup(d);
    }

    ~buffered_file_t() {
        if (this->data != nullptr && this->copied) {
            free(this->data);
        }
    }
    
    int open() override {
        this->filePointer = 0;
        return 0; // Not required for a memory based file
    }

    int close() override {
        free(this->data);
        this->data = nullptr;
        this->dataSize = 0;
        this->filePointer = 0;
        
        return 0;
    }

    int seek(unsigned int pos) override {
        if (pos >= dataSize) { this->filePointer = this->dataSize; return -1; }
        else {
            this->filePointer = pos;
            return 0;
        }
    }

    size_t tell() override {
        return this->filePointer;
    }

    size_t available() override {
        if (this->filePointer >= this->dataSize) { return 0; }
        else { return this->dataSize - this->filePointer; }
    }

    int read() override {
        return this->available() > 0 ? data[filePointer++] : -1;
    }

    buffered_file_t& operator=(const buffered_file_t& other) {
        if (this->data != nullptr && this->copied) {
            free(this->data);
        }

        this->data = other.data;
        this->dataSize = other.dataSize;
        this->copied = false;
        this->filePointer = 0;

        return *this;
    }
};

#endif