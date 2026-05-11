#ifndef LANGFILES_UTILITY_TYPE_CONVERTER_HPP__
#define LANGFILES_UTILITY_TYPE_CONVERTER_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "../settings.hpp"

namespace type_converter_helper {
    bool is_string(const char* str);
    bool is_number(const char* str);
    bool is_array(const char* str);

    void convert_string(double num, char* str, uint32_t str_size);
    double convert_double(const char* str);
    int convert_int(const char* str);
};

inline bool nearly_equal(double v1, double v2, double epsilon = 1e-9) { return fabs(v1 - v2) < epsilon; }

#endif