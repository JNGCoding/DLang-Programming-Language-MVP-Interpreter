#include "variable.hpp"

uint16_t calc_type(const char* str) {
    if (strcmp(str, VNUMBER_T) == 0) {
        return core_variable_types::Number;
    } else if (strcmp(str, VSTRING_T) == 0) {
        return core_variable_types::String;
    } else {
        return core_variable_types::InvalidVariable;
    }
}

uint8_t calc_comp_oper(const char* str) {
    if (strcmp(str, CEQUAL) == 0) {
        return comparison_operator::EqualTo;
    } else if (strcmp(str, CGREATERTHANEQUAL) == 0) {
        return comparison_operator::GreaterThanEqualTo;
    } else if (strcmp(str, CGREATERTHAN) == 0) {
        return comparison_operator::GreaterThan;
    } else if (strcmp(str, CLOWERTHANEQUAL) == 0) {
        return comparison_operator::LowerThanEqualTo;
    } else if (strcmp(str, CLOWERTHAN) == 0) {
        return comparison_operator::LowerThan;
    } else if (strcmp(str, CNOTEQUAL) == 0) {
        return comparison_operator::NotEqualTo;
    } else {
        return comparison_operator::NoOperator;
    }
}

static char small_buffer[MIN_STRING_SPACE];
static constexpr size_t small_buffer_size = MIN_STRING_SPACE;

//! ---------------------------------------------------
//~            STRING VARIABLE FUNCTIONS START
//! ---------------------------------------------------
int string_variable_t::initialize(size_t max_size) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        char** cdata = static_cast<char**>(this->value);
        cdata = new char*[max_size];
        if (!cdata) {
            delete[] cdata;
            return MEMORY_CORRUPTION;
        }

        this->value = cdata;

        this->count_size = max_size;
        this->var_defined = true;

    } else {

        this->var_defined = false;
        this->count_size = 1;

    }

    return PASS;
}

int string_variable_t::change_working_index(size_t ni) {
    if ( ni < this->count_size ) {
        working_index = ni;
    } else return BUFFER_OVERFLOW;

    return PASS;
}

int string_variable_t::set_data(char* data) {
    if (this->check_for_modifier(core_variable_modifiers::Normal)) {

        char* cdata = static_cast<char*>(this->value);
        if (this->var_defined) {
            delete[] cdata;
        }

        cdata = new (std::nothrow) char[ strlen(data) + 1 ];
        if (!cdata) {
            delete[] cdata;
            return MEMORY_CORRUPTION;
        }

        strcpy(cdata, data);
        this->var_defined = true;

        this->value = cdata;
        this->count_size = strlen(cdata);

    } else if (this->check_for_modifier(core_variable_modifiers::Array)) {

            // size_t appindex = 0;
            // char buffer[MIN_STRING_SPACE] = {0};

            // this->working_index = 0;

            // for (size_t i = 1; i < strlen(data) && this->working_index < this->count_size; i++) {
            //     char c = data[i];
            //     if ((c >= '0' && c <= '9') || (c == '.') || (c == '-')) {
            //         if (appindex < MIN_STRING_SPACE - 1) buffer[appindex++] = c;
            //     } else if (c == '|' || c == '}') {
            //         buffer[appindex] = '\0';

            //         int ci_res = this->change_index(buffer);
            //         if (ci_res != PASS) return ci_res;
            //         this->working_index++;

            //         // Reset buffer
            //         appindex = 0;
            //         buffer[0] = '\0';
            //     } else {
            //         return TYPE_MISMATCH;
            //     }

        if (type_converter_helper::is_array(data)) {
            size_t app_index = 0;

            size_t buffer_size = 50;
            char* buffer = static_cast<char*>(calloc(buffer_size, 1));
            if (buffer == nullptr) {
                return MEMORY_CORRUPTION;
            }

            size_t len = strlen(data);

            this->working_index = 0;
            for (size_t i = 1;  i < len && this->working_index < this->count_size; i++) {
                char c = data[i];

                if (c != '"' && c != '|' && c != '}') {
                    if (app_index < buffer_size) {
                        buffer[app_index++] = c;
                    } else {
                        buffer_size += 50;

                        char* tmp = static_cast<char*>(realloc(buffer, buffer_size));
                        if (tmp == nullptr) {
                            free(buffer);
                            return MEMORY_CORRUPTION;
                        }
                        buffer = tmp;

                        buffer[app_index++] = c;
                    }
                } else if (c == '|' || c == '}') {
                    if (app_index < buffer_size) {
                        buffer[app_index++] = '\0';
                    } else {
                        buffer_size += 1;

                        char* tmp = static_cast<char*>(realloc(buffer, buffer_size));
                        if (tmp == nullptr) {
                            free(buffer);
                            return MEMORY_CORRUPTION;
                        }
                        buffer = tmp;

                        buffer[app_index++] = '\0';
                    }

                    int ci_res = this->change_index(buffer);
                    if (ci_res != PASS) return ci_res;
                    this->working_index++;

                    app_index = 0;
                    buffer[0] = '\0';

                    if (c == '}') break;
                }
            }

            free(buffer);
        } else return TYPE_MISMATCH;

    } else return TYPE_MISMATCH;

    return PASS;
}

int string_variable_t::set_type(uint16_t newType) {
    if (this->check_for_modifier(core_variable_modifiers::Array)) {
        return TYPE_MISMATCH;
    } else {
        if (this->type_id == newType) return PASS;

        else if (newType == core_variable_types::Number) {
            if (this->type_id == core_variable_types::String) {
                if (!this->var_defined) {

                    this->type_id = newType;

                } else {

                    char* cdata = static_cast<char*>(this->value);
                    double value = 0;
                    for (size_t i = 0; i < strlen(cdata); i++) {
                        value += static_cast<double>(cdata[i]);
                    }

                    // delete previous string value
                    this->clear_var();
                    delete[] cdata;

                    // initialize the new variable with new value for double
                    this->value = new (std::nothrow) double(value);
                    if (this->value == nullptr) {
                        return MEMORY_CORRUPTION;
                    }
                }

            } else return TYPE_MISMATCH;
        } else return TYPE_MISMATCH;
    }

    return PASS;
}

char* string_variable_t::to_string() {
    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        if (this->var_defined) {
            return static_cast<char*>(this->value);
        } else {
            snprintf(small_buffer, small_buffer_size, "$null");
            return small_buffer;
        }

    } else if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        snprintf(small_buffer, small_buffer_size, "0x%p", this->value);
        return small_buffer;

    } else {

        snprintf(small_buffer, small_buffer_size, "error: unknown modifier on variable");
        return small_buffer;

    }
}

int string_variable_t::copy_index(char* str) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) {
        char** cdata = static_cast<char**>(this->value);

        if (cdata[this->working_index] != nullptr) {
            strcpy(str, cdata[this->working_index]);
        } else {
            strcpy(str, "[]null");
            return NOT_DEFINED;
        }

    } else {

        if (!this->var_defined) {
            return NOT_DEFINED;
        } else {
            str[0] = static_cast<char*>(this->value)[0];
            str[1] = '\0';
        }

    }

    return PASS;
}

int string_variable_t::change_index(char* str) {

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        char* cdata = static_cast<char*>(this->value);
        if (cdata != nullptr) {
            cdata[this->working_index] = str[0];
        } else return NOT_DEFINED;

    } else if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        char** cdata = static_cast<char**>(this->value);
        
        if (cdata[this->working_index] != nullptr) {
            delete[] cdata[this->working_index];
        }
    
        cdata[this->working_index] = new (std::nothrow) char[ strlen(str) + 1 ];
        if (cdata == nullptr) {
            delete[] cdata[this->working_index];
            return MEMORY_CORRUPTION;
        }
        strcpy(cdata[this->working_index], str);

    }

    return PASS;
}

char* string_variable_t::get_index_as_string() {
    if (this->check_for_modifier(core_variable_modifiers::Array)) {
        char* result = static_cast<char**>(this->value)[this->working_index];
        if (result != nullptr) return result;

        snprintf(small_buffer, small_buffer_size, "[]null");
        return small_buffer;
    } else {
        small_buffer[0] = static_cast<char*>(this->value)[this->working_index];
        small_buffer[1] = '\0';
        return small_buffer;
    }
}

int string_variable_t::clear_var() {
    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {
        if (this->var_defined) {
            this->set_data( const_cast<char*>("") );
        } else return PASS;
    } else if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        char** cdata = static_cast<char**>(this->value);
        for (size_t i = 0; i < this->count_size; i++) {
            if (cdata[i] != nullptr) { delete[] cdata[i]; }
        }

    } else return UNKNOWN_ERROR;

    return PASS;
}

size_t string_variable_t::get_size() {
    size_t result = VARIABLE_METADATA_SIZE;

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {
        if (this->var_defined) {
            result += strlen( static_cast<char*>(this->value) );
        } else return 0;
    } else if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        char** cdata = static_cast<char**>(this->value);
        for (size_t i = 0; i < this->count_size; i++) {
            if (cdata[i] != nullptr) { result += strlen(cdata[i]); }
        }

    } else return 0;

    return result;
}

int string_variable_t::add(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {
        char* cdata = static_cast<char*>(this->value);

        size_t l1 = strlen(cdata);
        size_t l2 = strlen(str);

        char* ndata = new char[ l1 + l2 + 1 ];

        strcat(ndata, cdata);
        strcat(ndata, str);

        delete[] cdata;

        this->value = ndata;

        return PASS;

    } else return NOT_DEFINED;
}

int string_variable_t::sub(char*) {
    return NOT_DEFINED;
}

int string_variable_t::div(char*) {
    return NOT_DEFINED;
}

int string_variable_t::mod(char*) {
    return NOT_DEFINED;
}

int string_variable_t::mul(char*) {
    return NOT_DEFINED;
}

int string_variable_t::bitwise_and(char*) {
    return NOT_DEFINED;
}

int string_variable_t::bitwise_or(char*) {
    return NOT_DEFINED;
}

int string_variable_t::bitwise_not() {
    return NOT_DEFINED;
}

int string_variable_t::bitwise_nand(char*) {
    return NOT_DEFINED;
}

int string_variable_t::bitwise_nor(char*) {
    return NOT_DEFINED;
}

int string_variable_t::bitwise_xor(char*) {
    return NOT_DEFINED;
}

int string_variable_t::bitwise_lshift(char*) {
    return NOT_DEFINED;
}

int string_variable_t::bitwise_rshift(char*) {
    return NOT_DEFINED;
}

bool string_variable_t::eq(char* str) {
    if ( !this->var_defined ) {
        return false;
    }

    char* cdata = static_cast<char*>(this->value);
    return strcmp(cdata, str) == 0;
}

bool string_variable_t::neq(char* str) {
    if ( !this->var_defined ) {
        return false;
    }

    char* cdata = static_cast<char*>(this->value);
    return strcmp(cdata, str) != 0;
}

bool string_variable_t::gteq(char* str) {
    if ( !this->var_defined ) {
        return false;
    }

    char* cdata = static_cast<char*>(this->value);
    return strlen(cdata) >= strlen(str);
}

bool string_variable_t::gt(char* str) {
    if ( !this->var_defined ) {
        return false;
    }

    char* cdata = static_cast<char*>(this->value);
    return strlen(cdata) > strlen(str);
}

bool string_variable_t::lteq(char* str) {
    if ( !this->var_defined ) {
        return false;
    }

    char* cdata = static_cast<char*>(this->value);
    return strlen(cdata) <= strlen(str);
}

bool string_variable_t::lt(char* str) {
    if ( !this->var_defined ) {
        return false;
    }

    char* cdata = static_cast<char*>(this->value);
    return strlen(cdata) < strlen(str);
}

//! ---------------------------------------------------
//~            NUMBER VARIABLE FUNCTIONS START
//! ---------------------------------------------------
int number_variable_t::initialize(size_t max_size) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        double* cdata = static_cast<double*>(this->value);
        cdata = new double[max_size]();
        if (!cdata) {
            delete[] cdata;
            return MEMORY_CORRUPTION;
        }

        this->value = cdata;

        this->count_size = max_size;
        this->var_defined = true;

    } else {

        this->value = new double(0);
        this->var_defined = true;

    }

    return PASS;
}

int number_variable_t::change_working_index(size_t ni) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) && ni < this->count_size ) {
        working_index = ni;
    } else if (ni >= this->count_size ){
        return BUFFER_OVERFLOW;
    } else { return TYPE_MISMATCH; }

    return PASS;
}

int number_variable_t::set_data(char* data) {

    if (this->check_for_modifier(core_variable_modifiers::Normal)) {

        if (type_converter_helper::is_number(data)) {
            double* cdata = static_cast<double*>(this->value);
            *cdata = atof(data);
        } else return TYPE_MISMATCH;
        // this->var_defined = true;

    } else if (this->check_for_modifier(core_variable_modifiers::Array)) {
        
        if (type_converter_helper::is_array(data)) {

            size_t appindex = 0;
            char buffer[MIN_STRING_SPACE] = {0};

            this->working_index = 0;

            size_t len = strlen(data);

            for (size_t i = 1; i < len && this->working_index < this->count_size; i++) {
                char c = data[i];
                if ((c >= '0' && c <= '9') || (c == '.') || (c == '-')) {
                    if (appindex < MIN_STRING_SPACE - 1) buffer[appindex++] = c;
                } else if (c == '|' || c == '}') {
                    buffer[appindex] = '\0';

                    int ci_res = this->change_index(buffer);
                    if (ci_res != PASS) return ci_res;
                    this->working_index++;

                    // Reset buffer
                    appindex = 0;
                    buffer[0] = '\0';

                    if (c == '}') break;
                } else return TYPE_MISMATCH;
            }

        } else return TYPE_MISMATCH;

    } else return TYPE_MISMATCH;

    return PASS;
}

int number_variable_t::set_type(uint16_t newType) {
    if (this->check_for_modifier(core_variable_modifiers::Array)) {
        return TYPE_MISMATCH;
    } else {
        if (this->type_id == newType) return PASS;

        else if (newType == core_variable_types::String) {
            if (this->type_id == core_variable_types::Number) {
                if (!this->var_defined) {

                    this->type_id = newType;

                } else {
                    double* cdata = static_cast<double*>(this->value);
                    double r = *cdata;
                    delete cdata;

                    this->value = new (std::nothrow) char[MIN_STRING_SPACE]();

                    if (this->value == nullptr) {
                        return MEMORY_CORRUPTION;
                    }

                    snprintf(static_cast<char*>(this->value), MIN_STRING_SPACE, "%f", r);
                }

            } else return TYPE_MISMATCH;
        } else return TYPE_MISMATCH;
    }

    return PASS;
}

char* number_variable_t::to_string() {
    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        if (this->var_defined) {
            double* cdata = static_cast<double*>(this->value);

            if ( static_cast<signed long long>(*cdata) != *cdata ) {
                snprintf(small_buffer, small_buffer_size, "%f", *cdata);
            } else {
                snprintf(small_buffer, small_buffer_size, "%d", static_cast<int>(*cdata));
            }

            return small_buffer;
        } else {
            snprintf(small_buffer, small_buffer_size, "$null");
            return small_buffer;
        }

    } else if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        snprintf(small_buffer, small_buffer_size, "0x%p", this->value);
        return small_buffer;

    } else {

        snprintf(small_buffer, small_buffer_size, "error: unknown modifier on variable");
        return small_buffer;

    }
}

int number_variable_t::copy_index(char* str) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        double* cdata = static_cast<double*>(this->value);
        snprintf(str, strlen(str), "%f", cdata[this->working_index]);
        return PASS;

    } else return TYPE_MISMATCH;
}

int number_variable_t::change_index(char* str) {

    if ( this->check_for_modifier(core_variable_modifiers::Array) && type_converter_helper::is_number(str)) {

        double* cdata = static_cast<double*>(this->value);
        cdata[this->working_index] = atof(str);

        return PASS;

    } else return TYPE_MISMATCH;
}

char* number_variable_t::get_index_as_string() {
    if (this->check_for_modifier(core_variable_modifiers::Array)) {
        double* cdata = static_cast<double*>(this->value);
        snprintf(small_buffer, small_buffer_size, "%f", cdata[this->working_index]);
        return small_buffer;
    } else {
        snprintf(small_buffer, small_buffer_size, "null");
    }

    return small_buffer;
}

int number_variable_t::clear_var() {
    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {
        if (this->var_defined) {
            double* cdata = static_cast<double*>(this->value);
            *cdata = 0;
        } else return PASS;
    } else if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        char** cdata = static_cast<char**>(this->value);
        for (size_t i = 0; i < this->count_size; i++) {
            if (cdata[i] != nullptr) { cdata[i] = 0; }
        }

    } else return UNKNOWN_ERROR;

    return PASS;
}

size_t number_variable_t::get_size() {
    size_t result = VARIABLE_METADATA_SIZE;

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        if (this->var_defined) {
            result += sizeof(double);
        } else return 0;

    } else if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

        result += sizeof(double) * this->count_size;

    } else return 0;

    return result;
}

int number_variable_t::add(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        double var2 = atof(str);

        *cdata += var2;

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::sub(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        double var2 = atof(str);

        *cdata -= var2;

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::div(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        double var2 = atof(str);

        *cdata /= var2;

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::mul(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        double var2 = atof(str);

        *cdata *= var2;

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::mod(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        int var2 = atoi(str);

        int result = static_cast<int>(*cdata) % var2;

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::bitwise_and(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        int var2 = atoi(str);

        int result = static_cast<int>(*cdata) & var2;

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::bitwise_or(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        int var2 = atoi(str);

        int result = static_cast<int>(*cdata) | var2;

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::bitwise_not() {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);

        int result = ~static_cast<int>(*cdata);

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::bitwise_nand(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        int var2 = atoi(str);

        int result = ~(static_cast<int>(*cdata) & var2);

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::bitwise_nor(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        int var2 = atoi(str);

        int result = ~(static_cast<int>(*cdata) | var2);

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::bitwise_xor(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        int var2 = atoi(str);

        int result = static_cast<int>(*cdata) ^ var2;

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::bitwise_lshift(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        int var2 = atoi(str);

        int result = static_cast<int>(*cdata) << var2;

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

int number_variable_t::bitwise_rshift(char* str) {
    if ( !this->var_defined ) {
        return MEMORY_CORRUPTION;
    }

    if ( !type_converter_helper::is_number(str) ) {
        return TYPE_MISMATCH;
    }

    if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

        double* cdata = static_cast<double*>(this->value);
        int var2 = atoi(str);

        int result = static_cast<int>(*cdata) >> var2;

        *cdata = static_cast<double>(result);

        return PASS;

    } else return NOT_DEFINED;
}

bool number_variable_t::eq(char* str) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) return false;

    if ( !this->var_defined ) {
        return false;
    }

    double* cdata = static_cast<double*>(this->value);

    return nearly_equal(*cdata, atof(str), 1e-6);
}

bool number_variable_t::neq(char* str) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) return false;

    if ( !this->var_defined ) {
        return false;
    }

    double* cdata = static_cast<double*>(this->value);

    return !nearly_equal(*cdata, atof(str), 1e-6);
}

bool number_variable_t::gteq(char* str) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) return false;

    if ( !this->var_defined ) {
        return false;
    }

    double* cdata = static_cast<double*>(this->value);

    return *cdata >= atof(str);
}

bool number_variable_t::gt(char* str) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) return false;

    if ( !this->var_defined ) {
        return false;
    }

    double* cdata = static_cast<double*>(this->value);

    return *cdata > atof(str);
}

bool number_variable_t::lteq(char* str) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) return false;

    if ( !this->var_defined ) {
        return false;
    }

    double* cdata = static_cast<double*>(this->value);

    return *cdata <= atof(str);
}

bool number_variable_t::lt(char* str) {
    if ( this->check_for_modifier(core_variable_modifiers::Array) ) return false;

    if ( !this->var_defined ) {
        return false;
    }

    double* cdata = static_cast<double*>(this->value);

    return *cdata < atof(str);
}