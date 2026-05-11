#include "type_converter.hpp"

namespace type_converter_helper {
    bool is_string(const char* str) {
        uint32_t str_size = strlen(str);
        if (str_size < 2) return false;

        return str[0] == '"' && str[str_size - 1] == '"';
    }

    bool is_number(const char* str) {
        if (str[0] == '\0') return false;
        if (str[0] == '.') return false;
        if (str[0] == '-' && str[1] == '.') return false;

        bool dot = false;

        for (unsigned int i = 0; str[i] != '\0' && i < MAX_STRING_SPACE; i++) {
            if (i == 0 && str[0] == '-') continue;

            if ((str[i] < '0' || str[i] > '9') && (str[i] != '.')) {
                return false;
            }

            if (str[i] == '.' && !dot) {
                dot = true;
            } else if (str[i] == '.' && dot) return false;
        }

        return true;
    }

    bool is_array(const char* str) {
        uint32_t str_size = strlen(str);
        if (str_size < 2) return false;

        return str[0] == '{' && str[str_size - 1] == '}';
    }

    void convert_string(double num, char* str, uint32_t str_size) {
        snprintf(str, str_size, "%f", num);
    }

    double convert_double(const char* str) {
        return atof(str);
    }

    int convert_int(const char* str) {
        return atoi(str);
    }
};