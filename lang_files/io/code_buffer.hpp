#ifndef LANGFILES_IO_CODE_BUFFER_HPP__
#define LANGFILES_IO_CODE_BUFFER_HPP__

#include <string.h>
#include <stdint.h>
#include "../settings.hpp"

class codefile_t {
public:
    codefile_t() {};
    virtual ~codefile_t() = default;
    virtual int open() = 0;
    virtual int close() = 0;
    virtual size_t tell() = 0;
    virtual int seek(unsigned int pos) = 0;
    virtual size_t available() = 0;
    virtual int read() = 0;
};

class codebuffer_t {
protected:
    size_t line_index = 0;
    codefile_t* file = nullptr;
    bool file_eof = true;
    char buffer[LINE_BUFFER_SIZE] = {0};
public:
    codebuffer_t() {}
    codebuffer_t(codefile_t* fp) : file(fp) {}

    void read_line();

    char* get_buffer() {
        return this->buffer;
    }

    bool eof() {
        return this->file_eof;
    }

    void load_file(codefile_t* fp) { this->file = fp; this->file_eof = false; }

    void unload_file() {
        if (this->file != nullptr) {
            this->file->close();
            this->file = nullptr;
        }

        this->line_index = 0;
        this->file_eof = true;
    };

    void set_line_index(size_t li) { this->line_index = li; }
    size_t get_line_index() { return this->line_index; }

    void set_char_index(size_t ci) {
        if (this->file != nullptr) {
            this->file->seek(ci);
            if (this->file->available() > 0) { this->file_eof = false; }
        }
    }

    size_t get_char_index() {
        if (this->file != nullptr) {
            return this->file->tell();
        } else return 0;
    }

    void set_eof(bool state) { this->file_eof = state; }
};

#endif