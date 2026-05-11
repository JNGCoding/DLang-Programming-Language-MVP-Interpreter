#ifndef LANGFILES_OBJECTS_FUNCTION_HPP__
#define LANGFILES_OBJECTS_FUNCTION_HPP__

#include <stdint.h>
#include <string.h>
#include <new>
#include "../instruction_codes.hpp"

static char small_buffer[MIN_STRING_SPACE];
static constexpr size_t small_buffer_size = MIN_STRING_SPACE;

enum core_function_types : uint16_t {
    Linespace,
    Function,
    InvalidFunction = 0xFFFF
};

#define FUNCTION_SIZE (sizeof(uint16_t) + (4 * sizeof(size_t)))

class function_t {
protected:
    uint16_t type_id = 0xFFFF;
    size_t schar_index = 0;
    size_t sline_index = 0;
    size_t echar_index = 0;
    size_t eline_index = 0;
    char* data = nullptr;
public:
    function_t() : type_id(core_function_types::Linespace) {}
    function_t(uint16_t tid, size_t schari) : type_id(tid), schar_index(schari) {}
    function_t(uint16_t tid, size_t schari, size_t slinei) : type_id(tid), schar_index(schari), sline_index(slinei) {}
    ~function_t() { if (this->data != nullptr) delete[] this->data; }

    void set_schar_index(size_t schar) { this->schar_index = schar; }
    void set_sline_index(size_t slinei) { this->sline_index = slinei; }
    void set_return_address(size_t echar) { this->echar_index = echar; }
    void set_return_line_index(size_t elinei) { this->eline_index = elinei; }

    uint16_t get_type() { return this->type_id; }
    size_t get_schar_index() { return this->schar_index; }
    size_t get_return_address() { return this->echar_index; }
    size_t get_sline_index() { return this->sline_index; }
    size_t get_return_line_index() { return this->eline_index; }

    size_t get_size() {
        size_t res = FUNCTION_SIZE;
        
        if (this->data != nullptr) {
            res += strlen(this->data);
        }

        return res;
    }

    int set_data(char* str) {
        if (str == nullptr) return PASS;

        size_t str_size = strlen(str);

        if (this->data == nullptr) {
            this->data = new (std::nothrow) char[str_size];
        } else {
            delete[] this->data;
            this->data = new (std::nothrow) char[str_size];
        }

        if (this->data == nullptr) {
            return MEMORY_CORRUPTION;
        }

        strcpy(this->data, str);

        return PASS;
    }

    char* return_data() {
        if (this->data == nullptr) {
            snprintf(small_buffer, small_buffer_size, "@null");
            return small_buffer;
        } else return this->data;
    }

    void delete_data() {
        if (this->data != nullptr) {
            delete[] this->data;
            this->data = nullptr;
        }
    }
};

#endif