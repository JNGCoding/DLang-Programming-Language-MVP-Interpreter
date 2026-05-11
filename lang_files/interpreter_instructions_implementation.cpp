#include "interpreter.hpp"

static char* current_word;

#define PARSE_NEXT_WORD if (this->tokenizer->end_of_line()) return INSUFFICIENT_ARGUMENTS; current_word = this->next(); if (this->check_and_raise_parse_error(current_word)) return PARSE_ERROR;

int BaseInterpreter::icomment() {
    return LINE_PARSED;
}

int BaseInterpreter::iprogramfinish() {
    if (this->tokenizer->end_of_line()) {
        // No error code and exit message is written, Fine we will exit with the default ones
        snprintf(this->exit_message, MAX_STRING_SPACE, "Program exited after encountering 'endprog' instruction at line %llu", this->code->get_line_index());
        this->exit(0, nullptr);
    } else {
        // error code is given
        current_word = this->next();
        if (this->check_and_raise_parse_error(current_word)) return PARSE_ERROR;
        if (!type_converter_helper::is_number(current_word)) return PARSE_ERROR;

        int ec = atoi(current_word);

        if (this->tokenizer->end_of_line()) {
            // We don't have exit message but that's fine, Just exit with the exit code
            snprintf(this->exit_message, MAX_STRING_SPACE, "Program exited after encountering 'endprog' instruction at line %llu", this->code->get_line_index());
            this->exit(ec, nullptr);
        } else {
            // Parse Exit message into current_word and exit
            current_word = this->next();
            if (this->check_and_raise_parse_error(current_word)) return PARSE_ERROR;
            this->exit(ec, current_word);
        }
    }

    return LINE_PARSED;
}

int BaseInterpreter::ierror() {
    // Parse Error Code
    PARSE_NEXT_WORD;
    
    if (!type_converter_helper::is_number(current_word)) return PARSE_ERROR;

    int ec = atoi(current_word);

    // Parse Error message
    PARSE_NEXT_WORD;

    // Provide Exit
    this->exit(ec, current_word);

    return LINE_PARSED;
}

int BaseInterpreter::imake() {
    PARSE_NEXT_WORD;

    core_variable_types type = static_cast<core_variable_types>(calc_type(current_word));

    PARSE_NEXT_WORD;
    
    core_variable_modifiers modifier = core_variable_modifiers::Normal;

    size_t var_size = 0;

    if (strcmp(current_word, "array") == 0) {
        modifier = core_variable_modifiers::Array;

        PARSE_NEXT_WORD;

        if (type_converter_helper::is_number(current_word)) {
            var_size = type_converter_helper::convert_int(current_word);
        } else return PARSE_ERROR;

        PARSE_NEXT_WORD;
    }

    int sv_res = this->store_variable(current_word, type, modifier, var_size);
    if (sv_res == PASS) {
        if (!this->tokenizer->end_of_line()) {
            variable_t* var = this->get_variable(current_word);

            PARSE_NEXT_WORD;

            int sd_res = var->set_data( current_word );
            switch (sd_res)
            {
            case MEMORY_CORRUPTION:
                snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: failed to create dataspace for variable at line %llu", this->code->get_line_index());
                this->exit(-1, nullptr);
                break;

            case TYPE_MISMATCH:
                snprintf(this->exit_message, MAX_STRING_SPACE, "type mismatch error: variable with type %u, modifier = %u doesn't support data '%s' at line %llu", var->get_type(), var->get_modifier(), current_word, this->code->get_line_index());
                this->exit(-1, nullptr);
                break;
            
            default:
                break;
            }
        }

    } else {
        #ifdef USING_DEBUG
        switch (sv_res)
        {
        case ALREADY_EXISTS:
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "imake() - variable '%s' alread exists at line %llu", current_word, this->code->get_line_index());
            this->debug.warning(this->log_message);
            break;
        
        default:
            break;
        }
        #endif
    }

    return LINE_PARSED;
}

int BaseInterpreter::ideclare() {
    PARSE_NEXT_WORD;

    if (strcmp(current_word, FFUNCTION_T) == 0) {
        PARSE_NEXT_WORD;

        char* function_name = strdup(current_word);

        size_t schar_index = this->code->get_char_index(), function_start = this->code->get_line_index();
        int s_res = this->walk_till_end_instruction(IFUNCTIONEND);
        if (s_res != PASS) {
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: def function instruction at line %llu is not ended with 'endfunc'", function_start);
            this->exit(-1, nullptr);
        }

        int sf_res = this->store_function(function_name, core_function_types::Function, schar_index, function_start);
        if (sf_res != PASS) {
            #ifdef USING_DEBUG
            switch (sf_res)
            {
            case ALREADY_EXISTS:

                snprintf(this->log_message, LOG_MESSAGE_SIZE, "ideclare() - function '%s' alread exists at line %llu", current_word, this->code->get_line_index());
                this->debug.warning(this->log_message);
                break;
            
            default:
                break;
            }
            #endif
        }

        // Important step: free function_name since its allocated in the heap
        free(function_name);

    } else if (strcmp(current_word, FLINESPACE_T) == 0) {
        PARSE_NEXT_WORD;

        int sf_res = this->store_function(current_word, core_function_types::Linespace, this->code->get_char_index(), this->code->get_line_index());
        if (sf_res != PASS) {
            #ifdef USING_DEBUG
            switch (sf_res)
            {
            case ALREADY_EXISTS:
                snprintf(this->log_message, LOG_MESSAGE_SIZE, "ideclare() - function '%s' alread exists at line %llu", current_word, this->code->get_line_index());
                this->debug.warning(this->log_message);
                break;
            
            default:
                break;
            }
            #endif
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: function type '%s' is not defined at line %llu", current_word, this->code->get_line_index());
    }

    return LINE_PARSED;
}

int BaseInterpreter::iset() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable(current_word);
    if (var != nullptr) {

        PARSE_NEXT_WORD;

        int sd_res = var->set_data( current_word );
        switch (sd_res)
        {
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: failed to create dataspace for variable at line %llu", this->code->get_line_index());
            this->exit(-1, nullptr);
            break;

        case TYPE_MISMATCH:
            snprintf(this->exit_message, MAX_STRING_SPACE, "type mismatch error: variable with type %u, modifier = %u doesn't support data '%s' at line %llu", var->get_type(), var->get_modifier(), current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }

    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::iinput() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable(current_word);
    if (var != nullptr) {
        char buffer[MAX_STRING_SPACE];
        this->input_stream->readString(buffer, MAX_STRING_SPACE);
    
        int sd_res = var->set_data(buffer);
        switch (sd_res)
        {
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: failed to create dataspace for variable at line %llu", this->code->get_line_index());
            this->exit(-1, nullptr);
            break;

        case TYPE_MISMATCH:
            snprintf(this->exit_message, MAX_STRING_SPACE, "type mismatch error: variable with type '%u' doesn't support data '%s' at line %llu", var->get_type(), current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return PASS;
}

int BaseInterpreter::iconvert() {
    PARSE_NEXT_WORD;

    // Parse type
    uint16_t type = calc_type(current_word);
    
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable(current_word);
    if (var != nullptr) {
        int st_res = var->set_type( type );
        switch (st_res)
        {
        case TYPE_MISMATCH:
            snprintf(this->exit_message, MAX_STRING_SPACE, "type mismatch error: variable '%s' cannot be converted to type %u at line %llu", current_word, type, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' failed to allocate new memory for type conversion at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;

        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::iclearvar() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable(current_word);
    if (var != nullptr) {
        var->clear_var();
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::isetindex() {
    PARSE_NEXT_WORD;

    if (!type_converter_helper::is_number(current_word)) return PARSE_ERROR;
    int index = atoi(current_word);

    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable(current_word);
    if (var != nullptr) {
        int cwi_res = var->change_working_index( index );
        switch (cwi_res) {
            case BUFFER_OVERFLOW:
                snprintf(this->exit_message, MAX_STRING_SPACE, "buffer overflow error: index %d out of range at line %llu", index, this->code->get_line_index());
                this->exit(-1, nullptr);
                break;

            case TYPE_MISMATCH:
                snprintf(this->exit_message, MAX_STRING_SPACE, "type mismatch error: variable with type '%u', modifier '%u' is not an array at line %llu", var->get_type(), var->get_modifier(), this->code->get_line_index());
                this->exit(-1, nullptr);
                break;

            default:
                break;
        }

        PARSE_NEXT_WORD;

        int ci_index = var->change_index(current_word);
        switch (ci_index) {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: variable data is null at line %llu", this->code->get_line_index());
            this->exit(-1, nullptr);
            break;

        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable failed to allocate new memory for new data at line %llu", this->code->get_line_index());
            break;

        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::igetindex() {
    PARSE_NEXT_WORD;

    if (!type_converter_helper::is_number(current_word)) return PARSE_ERROR;
    int index = atoi(current_word);

    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable(current_word);
    if (var != nullptr) {
        int cwi_res = var->change_working_index( index );
        switch (cwi_res) {
            case BUFFER_OVERFLOW:
                snprintf(this->exit_message, MAX_STRING_SPACE, "buffer overflow error: index %d out of range at line %llu", index, this->code->get_line_index());
                this->exit(-1, nullptr);
                break;

            case TYPE_MISMATCH:
                snprintf(this->exit_message, MAX_STRING_SPACE, "type mismatch error: variable with type '%u', modifier '%u' is not an array at line %llu", var->get_type(), var->get_modifier(), this->code->get_line_index());
                this->exit(-1, nullptr);
                break;

            default:
                break;
        }

        PARSE_NEXT_WORD;
        
        variable_t* dest = this->get_variable(current_word);
        if (dest != nullptr) {
            int sd_res = dest->set_data( var->get_index_as_string() );
            switch (sd_res) {
            case MEMORY_CORRUPTION:
                snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: failed to create dataspace for variable at line %llu", this->code->get_line_index());
                this->exit(-1, nullptr);
                break;

            case TYPE_MISMATCH:
                snprintf(this->exit_message, MAX_STRING_SPACE, "type mismatch error: variable with type '%u' doesn't support data '%s' at line %llu", var->get_type(), current_word, this->code->get_line_index());
                this->exit(-1, nullptr);
                break;
            
            default:
                break;
            }
        } else {
            snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::idel() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable(current_word);
    if (var != nullptr) {
        int rmv_res = this->remove_variable(current_word);
        switch (rmv_res)
        {
        case NOT_DEFINED:
            #ifdef USING_DEBUG
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "idel() - variable '%s' is not defined, remove operation failed", current_word);
            this->debug.warning(this->log_message);
            #endif
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::idelfunc() {
    PARSE_NEXT_WORD;

    function_t* func = this->get_function(current_word);
    if (func != nullptr) {
        int rmv_res = this->remove_function(current_word);
        switch (rmv_res)
        {
        case NOT_DEFINED:
            #ifdef USING_DEBUG
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "idelfunc() - function '%s' is not defined, remove operation failed", current_word);
            this->debug.warning(this->log_message);
            #endif
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: function '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::iprint() {
    while (!this->tokenizer->end_of_line()) {
        current_word = this->next();
        if (this->check_and_raise_parse_error(current_word)) return PARSE_ERROR;
        
        this->output_stream->print(current_word);
    }

    return LINE_PARSED;
}

int BaseInterpreter::iprintln() {
    this->iprint();
    this->output_stream->println("");

    return LINE_PARSED;
}

int BaseInterpreter::iadd() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->add( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;

        case TYPE_MISMATCH:
            snprintf(this->exit_message, MAX_STRING_SPACE, "type mismatch error: variable '%s' operation is giving type mismatch error at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::isub() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->sub( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::idiv() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->div( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::imul() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->mul( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::imod() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->mod( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::ior() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->bitwise_or( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::iand() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->bitwise_and( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::inot() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        int oper_res = var->bitwise_not();
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::inor() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->bitwise_nor( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::inand() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->bitwise_nand( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::ixor() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->bitwise_xor( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::ilshift() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->bitwise_lshift( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::irshift() {
    PARSE_NEXT_WORD;

    variable_t* var = this->get_variable( current_word );
    if (var != nullptr) {
        PARSE_NEXT_WORD;
        int oper_res = var->bitwise_rshift( current_word );
        switch (oper_res)
        {
        case NOT_DEFINED:
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operation not defined for variable '%s' with type %u at line %llu", current_word, var->get_type(), this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: variable '%s' is null at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
            break;
        
        default:
            break;
        }
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "error: variable '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::icall() {
    PARSE_NEXT_WORD;

    function_t* func = this->get_function(current_word);
    if (func != nullptr && func->get_type() == core_function_types::Function) {
        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "icall() - calling %s(return_address = %llu, return_line_index = %llu) at address = 0x%p", current_word, this->code->get_char_index(), this->code->get_line_index(), (void*) func);
            this->debug.message(this->log_message);
        }
        #endif

        func->set_return_address(this->code->get_char_index());
        func->set_return_line_index(this->code->get_line_index());

        int as_res = this->add_scope();
        switch (as_res) {
        case BUFFER_OVERFLOW:
            snprintf(this->exit_message, MAX_STRING_SPACE, "recursion error: max recursion depth %u reached at line %llu", this->current_scope - 1, this->code->get_line_index());
            this->exit(-1, nullptr);
            return LINE_PARSED;
        
        case MEMORY_CORRUPTION:
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: failed to allocate new memory for allocating new scope at line %llu", this->code->get_line_index());
            this->exit(-1, nullptr);
            return LINE_PARSED;
        
        default:
            break;
        }

        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "icall() - moving to char_index = %llu and line_index = %llu from char_index = %llu and line_index = %llu", func->get_schar_index(), func->get_sline_index(), this->code->get_char_index(), this->code->get_line_index());
            this->debug.message(this->log_message);
        }
        #endif

        this->code->set_char_index(func->get_schar_index());
        this->code->set_line_index(func->get_sline_index());

        this->call_stack.push(func);

        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "icall() - Appending new address from this->callstack: ");

            if (!this->call_stack.isEmpty()) {
                size_t len = strlen(this->log_message);
                snprintf(this->log_message + len, LOG_MESSAGE_SIZE - len, "stack<>(");

                for (size_t i = 0; i < this->call_stack.getSize() - 1; i++) {
                    len = strlen(this->log_message);
                    snprintf(this->log_message + len, LOG_MESSAGE_SIZE - len, "0x%p, ", (void*) *this->call_stack.get(i));
                }

                len = strlen(this->log_message);
                snprintf(this->log_message + len, LOG_MESSAGE_SIZE - len, "0x%p)", (void*) *this->call_stack.peek());
            } else {
                size_t len = strlen(this->log_message);
                snprintf(this->log_message + len, LOG_MESSAGE_SIZE - len, "stack<>()");
            }

            this->debug.message(this->log_message);
        }
        #endif

        this->current_running_function = func;
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: function '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return LINE_PARSED;
}

int BaseInterpreter::imove() {
    PARSE_NEXT_WORD;

    function_t* func = this->get_function(current_word);

    if (func != nullptr && func->get_type() == core_function_types::Linespace) {

        if (this->current_running_function != nullptr) {
            function_t* po_linespace = this->get_function(current_word, CURRENT);
            if (po_linespace == nullptr) {
                #ifdef USING_DEBUG
                {
    
                    snprintf(this->log_message, LOG_MESSAGE_SIZE, "imove() - calling 'iendfunc()', detected a 'move' instruction pointing to linespace '%s' outside of current running function at line %llu", current_word, this->code->get_line_index());
                    this->debug.warning(this->log_message);
                }
                #endif

                this->iendfunc();
            }
        }

        this->code->set_char_index(func->get_schar_index());
        this->code->set_line_index(func->get_sline_index());

        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "imove() - moving to char_index = %llu and line_index = %llu", this->code->get_char_index(), this->code->get_line_index());
            this->debug.message(this->log_message);
        }
        #endif
    } else {
        snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: function '%s' is not defined at line %llu", current_word, this->code->get_line_index());
        this->exit(-1, nullptr);
    }

    return PASS;
}

int BaseInterpreter::iendfunc() {
    if (this->current_running_function != nullptr && this->current_scope > 0) {
        // Return to the char index that called us
        this->code->set_char_index(this->current_running_function->get_return_address());
        this->code->set_line_index(this->current_running_function->get_return_line_index());

        if (!this->tokenizer->end_of_line()) {
            current_word = this->next();
            if (this->check_and_raise_parse_error(current_word)) return PARSE_ERROR;

            #ifdef USING_DEBUG
            {

                snprintf(this->log_message, LOG_MESSAGE_SIZE, "iendfunc() - setting function data to %s", current_word);
                this->debug.message(this->log_message);
            }
            #endif

            this->current_running_function->set_data(current_word);
        }

        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "iendfunc() - moving to char_index = %llu and line_index = %llu", this->code->get_char_index(), this->code->get_line_index());
            this->debug.message(this->log_message);
        }
        #endif

        this->remove_scope();

        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "iendfunc() - exiting function 0x%p", (void*) this->current_running_function);
            this->debug.message(this->log_message);
        }
        #endif

        this->call_stack.pop();
        function_t** ret_func = this->call_stack.peek();

        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "iendfunc() - Tried to pop address from this->callstack: ");

            if (!this->call_stack.isEmpty()) {
                size_t len = strlen(this->log_message);
                snprintf(this->log_message + len, LOG_MESSAGE_SIZE - len, "stack<>(");

                for (size_t i = 0; i < this->call_stack.getSize() - 1; i++) {
                    len = strlen(this->log_message);
                    snprintf(this->log_message + len, LOG_MESSAGE_SIZE - len, "0x%p, ", (void*) *this->call_stack.get(i));
                }

                len = strlen(this->log_message);
                snprintf(this->log_message + len, LOG_MESSAGE_SIZE - len, "0x%p)", (void*) *this->call_stack.peek());
            } else {
                size_t len = strlen(this->log_message);
                snprintf(this->log_message + len, LOG_MESSAGE_SIZE - len, "stack<>()");
            }

            this->debug.message(this->log_message);
        }
        #endif

        if (ret_func != nullptr) {
            this->current_running_function = *ret_func;
        } else this->current_running_function = nullptr;

        #ifdef USING_DEBUG
        {
            if (this->current_running_function != nullptr) {

                snprintf(this->log_message, LOG_MESSAGE_SIZE, "iendfunc() - exited to another function 0x%p", (void*) this->current_running_function);
                this->debug.message(this->log_message);
            }
        }
        #endif
    } else {
        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "iendfunc() - encountered 'endfunc' instruction but current_running_function is nullptr, scope_level = %u", this->current_scope);
            this->debug.warning(this->log_message);
        }
        #endif
    }

    return LINE_PARSED;
}

int BaseInterpreter::iif() {
    bool result = false;

    auto modify_result = [&](char append_oper, bool append_bool) {
        if (append_oper == '|') {
            result |= append_bool;
        } else {
            result &= append_bool;
        }
    };

    auto parse_condition = [&](char append_oper) {
        PARSE_NEXT_WORD;
    
        char* var1 = strdup(current_word);
        if (var1 == nullptr) {
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: failed to duplicate token [expr1] for comparison purposes at line %llu", this->code->get_line_index());
            this->exit(-1, nullptr);
            return LINE_PARSED;
        }
    
        PARSE_NEXT_WORD;
    
        uint8_t c_oper = calc_comp_oper(current_word);
        if (c_oper == comparison_operator::NoOperator) {
            free(var1);
    
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: operator '%s' is not defined at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
    
            return NOT_DEFINED;
        }
        
        PARSE_NEXT_WORD;
    
        char* var2 = strdup(current_word);
        if (var2 == nullptr) {
            free(var1);
            snprintf(this->exit_message, MAX_STRING_SPACE, "memory corruption error: failed to duplicate token [expr2] for comparison purposes at line %llu", this->code->get_line_index());
            this->exit(-1, nullptr);
            return LINE_PARSED;
        }
    
        if ( type_converter_helper::is_number(var1) && type_converter_helper::is_number(var2) ) {
            double v1 = type_converter_helper::convert_double(var1);
            double v2 = type_converter_helper::convert_double(var2);
    
            if (c_oper == comparison_operator::EqualTo) {
                modify_result(append_oper, v1 == v2);
            } else if (c_oper == comparison_operator::GreaterThanEqualTo) {
                modify_result(append_oper, v1 >= v2);
            } else if (c_oper == comparison_operator::GreaterThan) {
                modify_result(append_oper, v1 > v2);
            } else if (c_oper == comparison_operator::LowerThanEqualTo) {
                modify_result(append_oper, v1 <= v2);
            } else if (c_oper == comparison_operator::LowerThan) {
                modify_result(append_oper, v1 < v2);
            } else if (c_oper == comparison_operator::NotEqualTo) {
                modify_result(append_oper, v1 != v2);
            }
        } else {
            if (c_oper == comparison_operator::EqualTo) {
                modify_result(append_oper, strcmp(var1, var2) == 0);
            } else if (c_oper == comparison_operator::NotEqualTo) {
                modify_result(append_oper, strcmp(var1, var2) != 0);
            } else {
                snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: comparison operator %u is not defined at line %llu", c_oper, this->code->get_line_index());
                this->exit(-1, nullptr);
                return NOT_DEFINED;
            }
        }
    
        free(var1);
        free(var2);

        return LINE_PARSED;
    };

    int pc_res = parse_condition('|');
    if (pc_res == NOT_DEFINED) return LINE_PARSED;
    while (!this->tokenizer->end_of_line()) {
        PARSE_NEXT_WORD;

        if (current_word[0] == '\0') continue;

        if (strcmp(current_word, "||") == 0) {

            pc_res = parse_condition('|');
            if (pc_res == NOT_DEFINED) return LINE_PARSED;
        
        } else if (strcmp(current_word, "&&") == 0) {

            pc_res = parse_condition('&');
            if (pc_res == NOT_DEFINED) return LINE_PARSED;
        
        } else {
            snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: comparison append operator '%s' is not defined at line %llu", current_word, this->code->get_line_index());
            this->exit(-1, nullptr);
        }
    }

    // Convert the if statement into a function
    // and call if the operation is true
    size_t schar_index = this->code->get_char_index(), function_start = this->code->get_line_index();
    int s_res = this->walk_till_end_instruction(IIFEND);
    if (s_res != PASS) {
        snprintf(this->exit_message, MAX_STRING_SPACE, "not defined error: if instruction at line %llu is not ended with 'endif'", function_start);
        this->exit(-1, nullptr);
    }

    if (result) {
        // Execute the if block
        // this->perform_block(schar_index, function_start, this->code->get_line_index());

        this->code->set_char_index(schar_index);
        this->code->set_line_index(function_start);

        #ifdef USING_DEBUG
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "iif() - moving to char_index = %llu and line_index = %llu", this->code->get_char_index(), this->code->get_line_index());
            this->debug.message(this->log_message);
        }
        #endif
    }

    return LINE_PARSED;
}

int BaseInterpreter::iendif() {
    return LINE_PARSED;
}
