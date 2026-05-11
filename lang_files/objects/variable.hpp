#ifndef LANGFILES_OBJECTS_VARIABLE_HPP__
#define LANGFILES_OBJECTS_VARIABLE_HPP__

#include <stdint.h>
#include <string.h>
#include "../utility/type_converter.hpp"
#include "../instruction_codes.hpp"
#include "../settings.hpp"
#include "../ds/list.hpp"
#include <new>
#include <math.h>

enum core_variable_types : uint16_t {
    Number,
    String,
    InvalidVariable = 0xFFFF
};

enum core_variable_modifiers : uint16_t {
    Normal = 1 << 0,
    Array = 1 << 1,
};

enum comparison_operator : uint8_t {
    EqualTo,
    GreaterThanEqualTo,
    GreaterThan,
    LowerThanEqualTo,
    LowerThan,
    NotEqualTo,
    NoOperator
};

#define VARIABLE_METADATA_SIZE (2 * sizeof(uint16_t) + sizeof(bool) + 2 * sizeof(size_t) + sizeof(void*))

class variable_t {
protected:
    uint16_t type_id = InvalidVariable;
    uint16_t modifiers = Normal;
    bool var_defined = false;

    void* value = nullptr;
    size_t working_index = 0;
    size_t count_size = 0;
public:
    variable_t(uint16_t tid) : type_id(tid), modifiers(core_variable_modifiers::Normal), var_defined(false) {}
    variable_t(uint16_t tid, uint16_t mods) : type_id(tid), modifiers(mods), var_defined(false) {}

    variable_t(variable_t& var) {
        this->type_id = var.type_id;
        this->var_defined = var.var_defined;
        this->value = var.value;
        this->working_index = var.working_index;
        this->count_size = var.count_size;
        this->modifiers = var.modifiers;
    }

    virtual ~variable_t() = default;

    uint16_t get_type() { return this->type_id; }
    uint16_t get_modifier() { return this->modifiers; }
    bool check_for_modifier(uint16_t mods) { return (this->modifiers & mods) != 0; }
    bool is_defined() { return this->var_defined; }
    size_t get_length() { return this->count_size; }
    variable_t* create_shallow_copy() { return this; }

    // data_set functions
    virtual int initialize(size_t max_size) = 0;
    virtual int change_working_index(size_t ni) = 0;
    virtual int set_data(char* data) = 0;
    virtual int set_type(uint16_t newType) = 0;
    virtual char* to_string() = 0;
    virtual int copy_index(char* str) = 0;
    virtual int change_index(char* str) = 0;
    virtual char* get_index_as_string() = 0;
    virtual int clear_var() = 0;
    virtual size_t get_size() = 0;

    // arithmetic operations
    virtual int add(char* str) = 0;
    virtual int sub(char* str) = 0;
    virtual int div(char* str) = 0;
    virtual int mul(char* str) = 0;
    virtual int mod(char* str) = 0;

    // bitwise operations
    virtual int bitwise_and(char* str) = 0;
    virtual int bitwise_or(char* str) = 0;
    virtual int bitwise_not() = 0;
    virtual int bitwise_nand(char* str) = 0;
    virtual int bitwise_nor(char* str) = 0;
    virtual int bitwise_xor(char* str) = 0;

    // bitwise_shift operations
    virtual int bitwise_lshift(char* str) = 0;
    virtual int bitwise_rshift(char* str) = 0;

    // comparison operations
    virtual bool eq(char* str) = 0;
    virtual bool neq(char* str) = 0;
    virtual bool gteq(char* str) = 0;
    virtual bool gt(char* str) = 0;
    virtual bool lteq(char* str) = 0;
    virtual bool lt(char* str) = 0;
};

class number_variable_t : public variable_t {
private:
public:
    number_variable_t(uint16_t mods) : variable_t(Number, mods) {}

    ~number_variable_t() {
        if (this->check_for_modifier(core_variable_modifiers::Normal)) {
            delete static_cast<double*>(this->value);
        } else {
            delete[] static_cast<double*>(this->value);
        }
    }

    // data_set functions    
    int initialize(size_t max_size) override;
    int change_working_index(size_t ni) override;
    int set_data(char* data) override;
    int set_type(uint16_t newType) override;
    char* to_string() override;
    int copy_index(char* str) override;
    int change_index(char* str) override;
    char* get_index_as_string() override;
    int clear_var() override;
    size_t get_size() override;

    // arithmetic operations
    int add(char* str) override;
    int sub(char* str) override;
    int div(char* str) override;
    int mul(char* str) override;
    int mod(char* str) override;

    // bitwise operations
    int bitwise_and(char* str) override;
    int bitwise_or(char* str) override;
    int bitwise_not() override;
    int bitwise_nand(char* str) override;
    int bitwise_nor(char* str) override;
    int bitwise_xor(char* str) override;

    // bitwise_shift operations
    int bitwise_lshift(char* str) override;
    int bitwise_rshift(char* str) override;    

    // comparison operators
    bool eq(char* str) override;
    bool neq(char* str) override;
    bool gteq(char* str) override;
    bool gt(char* str) override;
    bool lteq(char* str) override;
    bool lt(char* str) override;
};

class string_variable_t : public variable_t {
private:
public:
    string_variable_t(uint16_t mods) : variable_t(String, mods) {}

    ~string_variable_t() {
        if ( this->check_for_modifier(core_variable_modifiers::Normal) ) {

            if (this->var_defined) {
                delete[] static_cast<char*>(this->value);
            }

        } else if ( this->check_for_modifier(core_variable_modifiers::Array) ) {

            char** cdata = static_cast<char**>(this->value);
            for (size_t i = 0; i < this->count_size; i++) {
                if (cdata[i] != nullptr) { delete[] cdata[i]; }
            }
            delete[] cdata;

        }
    }

    // data_set functions    
    int initialize(size_t max_size) override;
    int change_working_index(size_t ni) override;
    int set_data(char* data) override;
    int set_type(uint16_t newType) override;
    char* to_string() override;
    int copy_index(char* str) override;
    int change_index(char* str) override;
    char* get_index_as_string() override;

    int clear_var() override;
    size_t get_size() override;

    // arithmetic operations
    int add(char* str) override;
    int sub(char* str) override;
    int div(char* str) override;
    int mul(char* str) override;
    int mod(char* str) override;

    // bitwise operations
    int bitwise_and(char* str) override;
    int bitwise_or(char* str) override;
    int bitwise_not() override;
    int bitwise_nand(char* str) override;
    int bitwise_nor(char* str) override;
    int bitwise_xor(char* str) override;

    // bitwise_shift operations
    int bitwise_lshift(char* str) override;
    int bitwise_rshift(char* str) override;

    // comparison operations
    bool eq(char* str) override;
    bool neq(char* str) override;
    bool gteq(char* str) override;
    bool gt(char* str) override;
    bool lteq(char* str) override;
    bool lt(char* str) override;
};

extern uint16_t calc_type(const char* str);
extern uint8_t calc_comp_oper(const char* str);

#endif