#include "interpreter.hpp"

int BaseInterpreter::store_variable(const char* name, const core_variable_types type, const core_variable_modifiers modifiers, const size_t max_size) {
    variable_t* var;
    int cr_res = PASS;

    if (this->scoped_variables.getSize() > 0) {

        vstorage_t** variables = this->scoped_variables.peek();
        if (variables != nullptr) {

            if ((*variables)->get(name) != nullptr) return ALREADY_EXISTS;

            if (type == core_variable_types::String) {
                var = new string_variable_t(modifiers);
            } else if (type == core_variable_types::Number) {
                var = new number_variable_t(modifiers);
            } else {
                return TYPE_MISMATCH;
            }

            var->initialize(max_size);

            (*variables)->push(name, var);
        }

    } else {

        if (this->variables.get(name) != nullptr) return ALREADY_EXISTS;

        // add variable to global scope
        if (type == core_variable_types::String) {
            var = new string_variable_t(modifiers);
        } else if (type == core_variable_types::Number) {
            var = new number_variable_t(modifiers);
        } else {
            return TYPE_MISMATCH;
        }

        var->initialize(max_size);

        this->variables.push(name, var);
    }

    #ifdef USING_DEBUG
    if (this->debug.log_level != logger::level::NoOutput)
    {
        snprintf(this->log_message, LOG_MESSAGE_SIZE, "store_variable(name = %s, type = %u, modifiers = %u, max_size = %llu) - success at scope %u, address = 0x%p", name, type, modifiers, max_size, this->current_scope, (void*) var);
        this->debug.message(this->log_message);
    }
    #endif

    return cr_res;
}

int BaseInterpreter::store_function(const char* name, const core_function_types type, const size_t schari, const size_t slinei) {
    function_t* func;

    if (this->scoped_functions.getSize() > 0) {

        fstorage_t** functions = this->scoped_functions.peek();
        if (functions != nullptr) {
            if ((*functions)->get(name) != nullptr) return ALREADY_EXISTS;
            func = new function_t(type, schari, slinei);
            (*functions)->push(name, func);
        }

    } else {

        if (this->functions.get(name) != nullptr) return ALREADY_EXISTS;
        func = new function_t(type, schari, slinei);
        this->functions.push(name, func);

    }

    #ifdef USING_DEBUG
    if (this->debug.log_level != logger::level::NoOutput)
    {
        snprintf(this->log_message, LOG_MESSAGE_SIZE, "store_function(name = %s, type = %u, schari = %llu, slinei = %llu) - success at scope %u, address = 0x%p", name, type, schari, slinei, this->current_scope, (void*) func);
        this->debug.message(this->log_message);
    }
    #endif

    return PASS;
}

variable_t* BaseInterpreter::get_variable(const char* name, const uint8_t scope) {
    if (scope == ALL) {

        const size_t scope_size = this->scoped_variables.getSize();

        if (scope_size > 0) {
            size_t i = this->scoped_variables.getSize() - 1;
            size_t iterations = 0;

            while (iterations < scope_size) {
                vstorage_t** variables = this->scoped_variables.get(i);
                if (variables != nullptr) {
                    variable_t** variable = (*variables)->get(name);
                    if (variable != nullptr) return (*variable);
                }
                i--;
                iterations++;
            }

            variable_t** var = this->variables.get(name);
            return var == nullptr ? nullptr : *var;    

        } else {

            variable_t** var = this->variables.get(name);
            return var == nullptr ? nullptr : *var;

        }

    } else if (scope == GLOBAL) {

        variable_t** var = this->variables.get(name);
        return var == nullptr ? nullptr : *var;

    } else if (scope == CURRENT) {

        const size_t scope_size = this->scoped_variables.getSize();

        if (scope_size > 0) {
            vstorage_t** variables = this->scoped_variables.peek();

            if (variables != nullptr) {
                variable_t** variable = (*variables)->get(name);
                if (variable != nullptr) return (*variable); else return nullptr;
            } else {
                #ifdef USING_DEBUG
                {
                    this->debug.error("get_variable() - scope_size > 0 but variables == nullptr");
                }
                #endif
            }

        } else return nullptr;

    }

    return nullptr;
}

function_t* BaseInterpreter::get_function(const char* name, const uint8_t scope) {
    if (scope == ALL) {

        const size_t scope_size = this->scoped_functions.getSize();

        if (scope_size > 0) {
            size_t i = this->scoped_functions.getSize() - 1;
            size_t iterations = 0;

            while (iterations < scope_size) {
                fstorage_t** functions = this->scoped_functions.get(i);
                if (functions != nullptr) {
                    function_t** function = (*functions)->get(name);
                    if (function != nullptr) return (*function);
                }
                i--;
                iterations++;
            }

            function_t** func = this->functions.get(name);
            return func == nullptr ? nullptr : *func;    

        } else {

            function_t** func = this->functions.get(name);
            return func == nullptr ? nullptr : *func;

        }

    } else if (scope == GLOBAL) {

        function_t** func = this->functions.get(name);
        return func == nullptr ? nullptr : *func;

    } else if (scope == CURRENT) {

        const size_t scope_size = this->scoped_functions.getSize();

        if (scope_size > 0) {
            fstorage_t** functions = this->scoped_functions.peek();

            if (functions != nullptr) {
                function_t** function = (*functions)->get(name);
                if (function != nullptr) return (*function); else return nullptr;
            } else {
                #ifdef USING_DEBUG
                if (this->debug.log_level != logger::level::NoOutput)
                {
                    this->debug.error("get_variable() - scope_size > 0 but variables == nullptr");
                }
                #endif
            }

        } else return nullptr;

    }

    return nullptr;
}

int BaseInterpreter::remove_variable(const char* name) {
    stack<BaseInterpreter::vstorage_t*> scvariables = this->scoped_variables;
    if (scvariables.getSize() > 0) {
        vstorage_t** variables = scvariables.pop();
        if (variables != nullptr) {

            variable_t** var = (*variables)->get(name);
            if (var != nullptr) {
                delete (*var);
                (*variables)->pop(name);
            } else {
                return NOT_DEFINED;
            }

        }
    } else {

        variable_t** var = this->variables.get(name);
        if (var != nullptr) {
            delete (*var);
            this->variables.pop(name);
        } else {
            return NOT_DEFINED;
        }

    }

    return PASS;
}

int BaseInterpreter::remove_function(const char* name) {
    stack<BaseInterpreter::fstorage_t*> scfunctions = this->scoped_functions;
    if (scfunctions.getSize() > 0) {
        fstorage_t** functions = scfunctions.pop();
        if (functions != nullptr) {
            
            function_t** func = (*functions)->get(name);
            if (func != nullptr) {
                delete (*func);
                (*functions)->pop(name);
            } else {
                return NOT_DEFINED;
            }

        }
    } else {

        function_t** func = this->functions.get(name);
        if (func != nullptr) {
            delete (*func);
            this->functions.pop(name);
        } else {
            return NOT_DEFINED;
        }

    }

    return PASS;
}

int BaseInterpreter::add_scope() {
    this->current_scope++;
    if (this->current_scope > MAX_SCOPE_LEVEL)
        return BUFFER_OVERFLOW;

    vstorage_t* local_vars = new (std::nothrow) vstorage_t(MAX_VARIABLE_SPACE);
    fstorage_t* local_funcs = new (std::nothrow) fstorage_t(MAX_FUNCTION_SPACE);
    
    if (local_vars == nullptr) {
        snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "memory corruption error: creation of local variable space turned out to be null at line %llu", this->code->get_line_index());
        this->exit(-1, nullptr);
        return MEMORY_CORRUPTION;
    }

    #ifdef USING_DEBUG
    if (this->debug.log_level != logger::level::NoOutput)
    {
        snprintf(this->log_message, LOG_MESSAGE_SIZE, "add_scope() - appending new scope variables from scope level %u", this->current_scope);
        this->debug.message(this->log_message);
    }
    #endif

    if (local_funcs == nullptr) {
        snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "memory corruption error: creation of local function space turned out to be null at line %llu", this->code->get_line_index());
        this->exit(-1, nullptr);
        return MEMORY_CORRUPTION;
    }
    
    #ifdef USING_DEBUG
    if (this->debug.log_level != logger::level::NoOutput)
    {
        snprintf(this->log_message, LOG_MESSAGE_SIZE, "add_scope() - appending new scope functions from scope level %u", this->current_scope);
        this->debug.message(this->log_message);
    }
    #endif

    bool success = this->scoped_variables.push(local_vars);
    if (!success) {
        snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "unknown error: failed to add append a new variable scope to the scope stack at line %llu", this->code->get_line_index());
        this->exit(-1, nullptr);
        return UNKNOWN_ERROR;
    }

    success = this->scoped_functions.push(local_funcs);
    if (!success) {
        snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "unknown error: failed to add append a new function scope to the scope stack at line %llu", this->code->get_line_index());
        this->exit(-1, nullptr);
        return UNKNOWN_ERROR;
    }

    return PASS;
}

int BaseInterpreter::remove_scope() {
    if (this->current_scope > 0) {
        vstorage_t** sc_vars = this->scoped_variables.pop();
        if (sc_vars == nullptr) {
            #ifdef USING_DEBUG
            if (this->debug.log_level != logger::level::NoOutput)
            {
                snprintf(this->log_message, LOG_MESSAGE_SIZE, "remove_scope() - unknown error: local variable space turned out to be null, scope_level = %u", this->current_scope);
                this->debug.error(this->log_message);
            }
            #endif
        } else {
            #ifdef USING_DEBUG
            if (this->debug.log_level != logger::level::NoOutput)
            {
    
                snprintf(this->log_message, LOG_MESSAGE_SIZE, "remove_scope() - deleting scope variables from scope level %u", this->current_scope);
                this->debug.message(this->log_message);
            }
            #endif
    
            for (Pair<char*, variable_t*>& item : **sc_vars) {
                variable_t* var = item.getValue();
                delete var;
            }
    
            delete *sc_vars;
        }
    
        fstorage_t** sc_funcs = this->scoped_functions.pop();
        if (sc_funcs == nullptr) {
            #ifdef USING_DEBUG
            if (this->debug.log_level != logger::level::NoOutput)
            {
                snprintf(this->log_message, LOG_MESSAGE_SIZE, "remove_scope() - unknown error: local function space turned out to be null, scope_level = %u", this->current_scope);
                this->debug.error(this->log_message);
            }
            #endif
        } else {
            #ifdef USING_DEBUG
            if (this->debug.log_level != logger::level::NoOutput)
            {
    
                snprintf(this->log_message, LOG_MESSAGE_SIZE, "remove_scope() - deleting scope functions from scope level %u", this->current_scope);
                this->debug.message(this->log_message);
            }
            #endif
    
            for (Pair<char*, function_t*>& item : **sc_funcs) {
                function_t* func = item.getValue();
                delete func;
            }
    
            delete *sc_funcs;
        }
    
        this->current_scope--;
    } else {
        #ifdef USING_DEBUG
        if (this->debug.log_level != logger::level::NoOutput)
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "remove_scope() - current scope if less than or equal to 0, scope_level = %u", this->current_scope);
            this->debug.warning(this->log_message);
        }
        #endif
    }
    
    return PASS;
}

char* BaseInterpreter::parse_word(const char* str) {
    static char _pw_buffer[MAX_STRING_SPACE];

    if (str[0] == '$') {
        int str_size = strlen(str);
        int dollars__ = 0;

        // Count '$'
        for (int i = 0; i < str_size; i++) {
            if (str[i] == '$') {
                dollars__++;
            } else break;
        }

        if (dollars__ > 3) {
            snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: unable to specify the scope to pick the variable from, word = %s", str);
            return _pw_buffer;
        }

        if (str[dollars__] == '\0') {
            snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: variable name not given, word = %s", str);
            return _pw_buffer;
        }

        snprintf(_pw_buffer, MAX_STRING_SPACE, "%s", str + dollars__);

        variable_t* var = nullptr;

        if (dollars__ == 1) {
            var = this->get_variable(_pw_buffer);
        } else if (dollars__ == 2) {
            var = this->get_variable(_pw_buffer, GLOBAL);
        } else if (dollars__ == 3) {
            var = this->get_variable(_pw_buffer, CURRENT);
        }

        if (var != nullptr) {
            return var->to_string();
        } else {
            snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: variable not found, word = %s", str);
        }
    } else if (str[0] == '"') {
        if (type_converter_helper::is_string(str)) {
            int str_size = strlen(str);
            snprintf(_pw_buffer, MAX_STRING_SPACE, "%.*s", str_size - 2, str + 1);
        } else {
            snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: is not a string, word = '%s'", str);
        }
    } else if (str[0] == '#') {
        if (str[1] == '\0') {
            snprintf(_pw_buffer, MAX_STRING_SPACE, "#");
        } else {
            int str_size = strlen(str);
            int hashtags__ = 0;

            for (int i = 0; i < str_size; i++) {
                if (str[i] == '#') {
                    hashtags__++;
                } else break;
            }

            if (hashtags__ > 3) {
                snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: unable to specify the scope to pick the variable from, word = %s", str);
                return _pw_buffer;
            }

            if (str[hashtags__] == '\0') {
                snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: variable name not given, word = %s", str);
                return _pw_buffer;
            }

            snprintf(_pw_buffer, MAX_STRING_SPACE, "%s", str + hashtags__);

            variable_t* var = nullptr;

            if (hashtags__ == 1) {
                var = this->get_variable(_pw_buffer);
            } else if (hashtags__ == 2) {
                var = this->get_variable(_pw_buffer, GLOBAL);
            } else if (hashtags__ == 3) {
                var = this->get_variable(_pw_buffer, CURRENT);
            }

            if (var != nullptr) {
                snprintf(_pw_buffer, MAX_STRING_SPACE, "%llu", var->get_length());
            } else {
                snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: variable not found, word = %s", str);
            }
        }
    } else if (str[0] == '@') {
        if (str[1] == '\0') {
            snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: function name not given, word = %s", str);
        } else {
            int str_size = strlen(str);
            int at_the_rate__ = 0;

            for (int i = 0; i < str_size; i++) {
                if (str[i] == '@') {
                    at_the_rate__++;
                } else break;
            }

            if (at_the_rate__ > 3) {
                snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: unable to specify the scope to pick the function from, word = %s", str);
                return _pw_buffer;
            }

            if (str[at_the_rate__] == '\0') {
                snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: function name not given, word = %s", str);
                return _pw_buffer;
            }

            snprintf(_pw_buffer, MAX_STRING_SPACE, "%s", str + at_the_rate__);

            function_t* func = nullptr;

            if (at_the_rate__ == 1) {
                func = this->get_function(_pw_buffer);
            } else if (at_the_rate__ == 2) {
                func = this->get_function(_pw_buffer, GLOBAL);
            } else if (at_the_rate__ == 3) {
                func = this->get_function(_pw_buffer, CURRENT);
            }

            if (func != nullptr) {
                snprintf(_pw_buffer, MAX_STRING_SPACE, "%s", func->return_data());
                func->delete_data();
            } else {
                snprintf(_pw_buffer, MAX_STRING_SPACE, "parsing error: function not found, word = %s", str);
            }
        }
    } else {
        snprintf(_pw_buffer, MAX_STRING_SPACE, str);
    }

    return _pw_buffer;
}

void BaseInterpreter::run_file() {
    if (this->tokenizer == nullptr) {
        snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "fatal error: tokenizer object not defined, this->tokenizer is null");
        this->exit(-1, nullptr);
        return;
    }

    if (this->output_stream == nullptr) {
        this->debug.error("run_file() - this->output_stream is null");
        this->debug.warning("run_file() - text output functions or even the program execution may not function properly");
    }

    if (this->input_stream == nullptr) {
        this->debug.error("run_file() - this->input_stream is null");
        this->debug.warning("run_file() - text input functions or even the program execution may not function properly");
    }

    this->run_code = true;
    this->script_mode = false;
    this->call_stack.clear();
    this->current_running_function = nullptr;

    this->clean_heap();

    #ifdef USING_DEBUG
    if (this->debug.get_wif_perm()) { logger_open_file(); }
    #endif

    while (!this->code->eof()) {
        if (!this->run_code) break;

        this->code->read_line();
        const char* code_line = this->code->get_buffer();
        if (code_line[0] == '\0') continue;

        this->perform_line(code_line);
    }

    if (this->run_code) this->exit(0, "Program exited normally after reading all instructions");
    this->run_code = false;

    #ifdef USING_DEBUG
    if (this->debug.log_level != logger::level::NoOutput)
    {
        struct BaseInterpreter::mem_usage s = this->get_approximate_heap_usage();

        this->debug.message("run_file() - program execution completed");

        snprintf(this->log_message, LOG_MESSAGE_SIZE, "run_file() - variable space memory usage: %llu", s.var_size);
        this->debug.message(this->log_message);

        snprintf(this->log_message, LOG_MESSAGE_SIZE, "run_file() - function space memory usage: %llu", s.func_size);
        this->debug.message(this->log_message);

        snprintf(this->log_message, LOG_MESSAGE_SIZE, "run_file() - total memory usage: %llu", s.func_size + s.var_size);
        this->debug.message(this->log_message);
    }

    if (this->debug.get_wif_perm()) {
        logger_close_file();
    }
    #endif

}

void BaseInterpreter::exit(int ec, const char* es) {
    if (this->script_mode) {
        if (es != nullptr) snprintf(this->exit_message, ERROR_MESSAGE_SIZE, es);
        this->output_stream->println(this->exit_message);
    } else {
        this->exit_code = ec;
        if (es != nullptr) snprintf(this->exit_message, ERROR_MESSAGE_SIZE, es);
        this->run_code = false;
    }
}

int BaseInterpreter::perform_block(size_t SCI, size_t SLI, size_t ELI) {
    this->code->set_char_index(SCI);
    this->code->set_line_index(SLI);

    for (size_t i = 0; i < ELI; i++) {
        if (!this->run_code || this->code->eof()) break;

        this->code->read_line();
        char* code_line = this->code->get_buffer();
        int res = this->perform_line(code_line);

        if (res != LINE_PARSED && res != CONTINUE_LINE) return res;
    }

    return LINE_PARSED;
}

int BaseInterpreter::perform_line(const char* str) {
    this->tokenizer->load_line_in_buffer(str);

    #ifdef USING_DEBUG
    if (this->debug.log_level != logger::level::NoOutput)
    {
        snprintf(this->log_message, LOG_MESSAGE_SIZE, "perform_line(%s)", str);
        this->debug.message(this->log_message);
    }
    #endif

    const char* primary_instruction = this->next();
    if (this->check_and_raise_parse_error(primary_instruction)) return PARSE_ERROR;

    snprintf(this->current_instruction, MID_STRING_SPACE, primary_instruction);

    interpreter_function<BaseInterpreter>* func = this->core_instructions.get(primary_instruction);
    int func_res = NOT_DEFINED;
    if (func != nullptr) {
        func_res = (this->**func)();
    } else {
        func_res = this->externals(primary_instruction);
    }

    switch (func_res)
    {
    case INSUFFICIENT_ARGUMENTS:
        snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "fatal error: insufficient arguments passed to the instruction '%s' at line %llu", current_instruction, this->code->get_line_index());
        this->exit(-1, nullptr);
        break;
    
    case NOT_DEFINED:
        snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "fatal error: instruction '%s' is not defined at line %llu", current_instruction, this->code->get_line_index());
        this->exit(-1, nullptr);
        break;

    default:
        break;
    }

    return func_res;
}

void BaseInterpreter::print_exit_message() {
    if (this->output_stream != nullptr) {
        char message[MIN_STRING_SPACE];
        snprintf(message, MIN_STRING_SPACE, "Exit Code: %d", this->exit_code);
        this->output_stream->println(message);
        this->output_stream->println(this->exit_message);
        this->output_stream->flush();
    }
}

int BaseInterpreter::clean_heap() {
    if (!this->scoped_variables.isEmpty()) {
        for (size_t i = 0; i < this->scoped_variables.getSize(); i++) {
            vstorage_t* sc_vars = *this->scoped_variables.pop();
            for (Pair<char*, variable_t*>& item : *sc_vars) {
                variable_t* var = item.getValue();
                delete var;
            }

            sc_vars->clear();
            delete sc_vars;
        }
    }

    this->scoped_variables.clear();

    for (Pair<char*, variable_t*>& item : this->variables) {
        variable_t* var = item.getValue();
        delete var;
    }

    this->variables.clear();

    if (!this->scoped_functions.isEmpty()) {
        for (size_t i = 0; i < this->scoped_functions.getSize(); i++) {
            fstorage_t* sc_funcs = *this->scoped_functions.pop();
            for (Pair<char*, function_t*>& item : *sc_funcs) {
                function_t* func = item.getValue();
                delete func;
            }

            sc_funcs->clear();
            delete sc_funcs;
        }
    }

    this->scoped_functions.clear();

    for (Pair<char*, function_t*>& item : this->functions) {
        function_t* func = item.getValue();
        delete func;
    }

    this->functions.clear();

    return PASS;
}

struct BaseInterpreter::mem_usage BaseInterpreter::get_approximate_heap_usage() {
    size_t var_storage = 0;
    size_t func_storage = 0;

    if (!this->scoped_variables.isEmpty()) {
        for (size_t i = 0; i < this->scoped_variables.getSize(); i++) {
            vstorage_t* sc_vars = *this->scoped_variables.pop();
            for (Pair<char*, variable_t*>& item : *sc_vars) {
                var_storage += item.getValue()->get_size();
            }
        }
    }

    for (Pair<char*, variable_t*>& item : this->variables) {
        var_storage += item.getValue()->get_size();
    }

    if (!this->scoped_functions.isEmpty()) {
        for (size_t i = 0; i < this->scoped_functions.getSize(); i++) {
            fstorage_t* sc_funcs = *this->scoped_functions.pop();
            for (Pair<char*, function_t*>& item : *sc_funcs) {
                func_storage += item.getValue()->get_size();
            }
        }
    }

    for (Pair<char*, function_t*>& item : this->functions) {
        func_storage += item.getValue()->get_size();
    }

    return BaseInterpreter::mem_usage{var_storage, func_storage};
}

int BaseInterpreter::walk_till_end_instruction(const char* end_inst) {
    bool present = false;

    #ifdef USING_DEBUG
    if (this->debug.log_level != logger::level::NoOutput)
    {
        snprintf(this->log_message, LOG_MESSAGE_SIZE, "walk_till_end_instruction(%s) - starting char_index = %llu and line_index = %llu", end_inst, this->code->get_char_index(), this->code->get_line_index());
        this->debug.message(this->log_message);
    }
    #endif    

    while (!this->code->eof()) {
        if (!this->run_code) break;

        this->code->read_line();
        const char* code_line = this->code->get_buffer();
        if (code_line[0] == '\0') continue;

        this->tokenizer->load_line_in_buffer(code_line);

        const char* primary_instruction = this->next();
        if (this->check_and_raise_parse_error(primary_instruction)) return PARSE_ERROR;

        #ifdef USING_DEBUG
        if (this->debug.log_level != logger::level::NoOutput)
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "walk_till_end_instruction(%s) - line = %s, instruction = %s", end_inst, code_line, primary_instruction);
            this->debug.message(this->log_message);
        }
        #endif

        if (strcmp(primary_instruction, IIF) == 0) {

            int s_res = this->walk_till_end_instruction(IIFEND);
            if (s_res != PASS) return s_res;

        } else if (strcmp(primary_instruction, IDECLARE) == 0) {
            
            const char* function_type = this->next();
            if (this->check_and_raise_parse_error(function_type)) return PARSE_ERROR;

            if (strcmp(function_type, FFUNCTION_T) == 0)
            {
                int s_res = this->walk_till_end_instruction(IFUNCTIONEND);
                if (s_res != PASS) return s_res;
            }

        } else if (strcmp(primary_instruction, end_inst) == 0) {

            present = true;
            break;

        }
    }

    #ifdef USING_DEBUG
    if (this->debug.log_level != logger::level::NoOutput)
    {
        snprintf(this->log_message, LOG_MESSAGE_SIZE, "walk_till_end_instruction(%s) - ending char_index = %llu and line_index = %llu", end_inst, this->code->get_char_index(), this->code->get_line_index());
        this->debug.message(this->log_message);
    }
    #endif

    return present ? PASS : NOT_DEFINED;
}

int BaseInterpreter::run_script_mode() {
    if (this->tokenizer == nullptr) {
        snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "fatal error: tokenizer object not defined, this->tokenizer is null");
        this->exit(-1, nullptr);

        return UNKNOWN_ERROR;
    }

    if (this->output_stream == nullptr) {
        this->debug.error("run_file() - this->output_stream is null");
        this->debug.warning("run_file() - text output functions or even the program execution may not function properly");
    }

    if (this->input_stream == nullptr) {
        this->debug.error("run_file() - this->input_stream is null");
        this->debug.warning("run_file() - text input functions or even the program execution may not function properly");

        return UNKNOWN_ERROR;
    }

    this->run_code = true;
    this->script_mode = true;
    this->call_stack.clear();
    this->current_running_function = nullptr;

    this->clean_heap();

    this->output_stream->print("ALPL (Assembly Like Programming Language) interpreter, version = ");
    this->output_stream->println(VERSION);
    this->output_stream->println("Created by Dhruv a.k.a JNGCoding on 22-04-2026");
    this->output_stream->println("Running in script-mode");
    this->output_stream->println("run exit/endprog instruction to exit.");

    char code_line[LINE_BUFFER_SIZE] = {0};
    
    while (true) {
        this->output_stream->print(">>> ");

        this->input_stream->readString(code_line, LINE_BUFFER_SIZE);
        if (code_line[0] == '\0') continue;

        this->tokenizer->load_line_in_buffer(code_line);

        #ifdef USING_DEBUG
        if (this->debug.log_level != logger::level::NoOutput)
        {
            snprintf(this->log_message, LOG_MESSAGE_SIZE, "run_script_mode() - %s", code_line);
            this->debug.message(this->log_message);
        }
        #endif

        const char* primary_instruction = this->next();
        if (this->check_and_raise_parse_error(primary_instruction)) return PARSE_ERROR;

        snprintf(this->current_instruction, MID_STRING_SPACE, primary_instruction);        

        if (strcmp(primary_instruction, "exit") == 0) {
            break;
        } else if (strcmp(primary_instruction, "endprog") == 0) {
            break;
        } else {
            interpreter_function<BaseInterpreter>* func = this->core_instructions.get(primary_instruction);
            int func_res = NOT_DEFINED;
            if (func != nullptr) {
                func_res = (this->**func)();
            } else {
                func_res = this->externals(primary_instruction);
            }
    
            switch (func_res)
            {
            case INSUFFICIENT_ARGUMENTS:
                snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "fatal error: insufficient arguments passed to the instruction '%s' at line %llu", current_instruction, this->code->get_line_index());
                this->exit(-1, nullptr);
                break;
            
            case NOT_DEFINED:
                snprintf(this->exit_message, ERROR_MESSAGE_SIZE, "fatal error: instruction '%s' is not defined at line %llu", current_instruction, this->code->get_line_index());
                this->exit(-1, nullptr);
                break;
    
            default:
                break;
            }
        }
    }

    this->run_code = false;

    return PASS;
}