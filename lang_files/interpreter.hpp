#ifndef LANGFILES_INTERPRETER_HPP__
#define LANGFILES_INTERPRETER_HPP__

#include "tokenizer/word_factory.hpp"

#include "io/code_buffer.hpp"
#include "io/input_stream.hpp"
#include "io/output_stream.hpp"

#include "instruction_codes.hpp"

#include "objects/variable.hpp"
#include "objects/function.hpp"
#include "objects/storage_group.hpp"

#include "settings.hpp"
#include "space_allocator.hpp"

#include "utility/logging.hpp"
#include "utility/type_converter.hpp"

#include "ds/stack.hpp"

#include <string.h>
#include <stdio.h>

// FOR EASE OF USE
typedef codebuffer_t fileloader;

class BaseInterpreter {
protected:
    template<typename T>
    using interpreter_function = int(T::*)(void);
    using vstorage_t = storage_group_t<variable_t*>;
    using fstorage_t = storage_group_t<function_t*>;
    using istorage_t = storage_group_t<interpreter_function<BaseInterpreter>>;
private:
    bool run_code = false;
    bool script_mode = false;
    int exit_code = 0;
    char exit_message[ERROR_MESSAGE_SIZE];
    char log_message[LOG_MESSAGE_SIZE];

    word_factory_t* tokenizer = nullptr;
    codebuffer_t* code = nullptr;

    input_stream_t* input_stream = nullptr;
    output_stream_t* output_stream = nullptr;

    vstorage_t variables = vstorage_t( MAX_VARIABLE_SPACE );
    fstorage_t functions = fstorage_t( MAX_FUNCTION_SPACE );
    istorage_t core_instructions = istorage_t( 64 ); // ! Hardcoded value because fuck it, my language, my rules

    logger debug = logger("interpreter");

    // variables for debugging and are integrated into the internal workings of interpreter
    char current_instruction[MID_STRING_SPACE];

    // Max level scope of MAX_SCOPE_LEVEL
    /*
    One advantage that we will now have is the moving to a linespace outside a function
    We can simply check if the current scope in the stack contains the linespace
    and if not found then we can simply delete the whole scope and get out of the function
    */
    stack<function_t*> call_stack = stack<function_t*>(MAX_SCOPE_LEVEL);
    stack<vstorage_t*> scoped_variables = stack<vstorage_t*>(MAX_SCOPE_LEVEL);
    stack<fstorage_t*> scoped_functions = stack<fstorage_t*>(MAX_SCOPE_LEVEL);

    uint8_t current_scope = 0;
    function_t* current_running_function = nullptr;

    void load_core_instructions() {
        this->core_instructions.push(ICOMMENT, BaseInterpreter::icomment);
        this->core_instructions.push(ISHORT_HAND_COMMENT, BaseInterpreter::icomment);
        this->core_instructions.push(IPROGRAMFINISH, BaseInterpreter::iprogramfinish);
        this->core_instructions.push(IERROR, BaseInterpreter::ierror);
        this->core_instructions.push(IMAKE, BaseInterpreter::imake);
        this->core_instructions.push(IDECLARE, BaseInterpreter::ideclare);
        this->core_instructions.push(ISET, BaseInterpreter::iset);
        this->core_instructions.push(IINPUT, BaseInterpreter::iinput);
        this->core_instructions.push(ICONVERT, BaseInterpreter::iconvert);
        this->core_instructions.push(ICLEARVAR, BaseInterpreter::iclearvar);
        this->core_instructions.push(ISETIDX, BaseInterpreter::isetindex);
        this->core_instructions.push(IGETIDX, BaseInterpreter::igetindex);
        this->core_instructions.push(IDELETE, BaseInterpreter::idel);
        this->core_instructions.push(IDELFUNC, BaseInterpreter::idelfunc);
        this->core_instructions.push(IPRINT, BaseInterpreter::iprint);
        this->core_instructions.push(IPRINTLN, BaseInterpreter::iprintln);
        this->core_instructions.push(IADD, BaseInterpreter::iadd);
        this->core_instructions.push(ISUBTRACT, BaseInterpreter::isub);
        this->core_instructions.push(IDIVIDE, BaseInterpreter::idiv);
        this->core_instructions.push(IMULTIPLY, BaseInterpreter::imul);
        this->core_instructions.push(IMOD, BaseInterpreter::imod);
        this->core_instructions.push(IOR, BaseInterpreter::ior);
        this->core_instructions.push(IAND, BaseInterpreter::iand);
        this->core_instructions.push(INOT, BaseInterpreter::inot);
        this->core_instructions.push(INOR, BaseInterpreter::inor);
        this->core_instructions.push(INAND, BaseInterpreter::inand);
        this->core_instructions.push(IXOR, BaseInterpreter::ixor);
        this->core_instructions.push(ILS, BaseInterpreter::ilshift);
        this->core_instructions.push(IRS, BaseInterpreter::irshift);
        this->core_instructions.push(ICALL, BaseInterpreter::icall);
        this->core_instructions.push(IMOVE, BaseInterpreter::imove);
        this->core_instructions.push(IFUNCTIONEND, BaseInterpreter::iendfunc);
        this->core_instructions.push(IIF, BaseInterpreter::iif);
        this->core_instructions.push(IIFEND, BaseInterpreter::iendif);
    }
protected:
    int store_variable(const char* name, const core_variable_types type, const core_variable_modifiers modifiers, const size_t max_size = 0);
    int store_function(const char* name, const core_function_types type, const size_t schari, const size_t slinei);

    variable_t* get_variable(const char* name, const uint8_t scope = ALL);
    function_t* get_function(const char* name, const uint8_t scope = ALL);

    int remove_variable(const char* name);
    int remove_function(const char* name);

    int add_scope();
    int remove_scope();
    
    char* parse_word(const char* str);
    int perform_line(const char* str);
    int perform_block(size_t SCI, size_t SLI, size_t ELI);

    int walk_till_end_instruction(const char* end_inst);

    char* next() {
        char* word = this->tokenizer->get_next_word();
        char* res = this->parse_word( word );

        #ifdef USING_DEBUG
        {
            char log[LOG_MESSAGE_SIZE];
            snprintf(log, LOG_MESSAGE_SIZE, "next() - word = %s, res = %s", word, res);
            this->debug.message(log);
        }
        #endif
        
        return res;
    }
    
    bool check_and_raise_parse_error(const char* str) {
        if (strstr(str, "parsing error") != NULL) {
            snprintf(this->exit_message, MAX_STRING_SPACE, "internal error: '%s' at line %llu", str, this->code->get_line_index());
            this->exit(-1, nullptr);
            return true;
        }
        
        return false;
    }
    
    enum allocator_ids : uint8_t {
        heap_allocated
    };

    int icomment();
    int iprogramfinish();
    int ierror();
    int imake();
    int ideclare();
    int iset();
    int iinput();
    int iconvert();
    int iclearvar();
    int isetindex();
    int igetindex();
    int idel();
    int idelfunc();
    int iprint();
    int iprintln();
    int iadd();
    int isub();
    int idiv();
    int imul();
    int imod();
    int ior();
    int iand();
    int inot();
    int inor();
    int inand();
    int ixor();
    int ilshift();
    int irshift();
    int icall();
    int imove();
    int iendfunc();
    int iif();
    int iendif();
public:
    BaseInterpreter() {
        this->load_core_instructions();
        this->run_code = false;
        this->script_mode = false;
        this->debug.change_log_level(logger::level::NoOutput);
    }

    bool allocate_buffers(size_t _mem_size = DEFAULT_MEM) {
        // pool_allocator::master_allocate(_mem_size);

        return true;
    }

    void set_streams(output_stream_t* out, output_stream_t* debug, input_stream_t* in) {
        this->output_stream = out;
        this->input_stream = in;
        this->debug.output = debug;  //& Resetting debug logger output stream
    }

    void set_tokenizer(word_factory_t* _tokenizer) { this->tokenizer = _tokenizer; }

    void set_debug_level(logger::level le) { this->debug.change_log_level(le); }
    void set_debug_write_file(const bool __wif) { this->debug.write_in_file(__wif); }

    void set_fileloader(codebuffer_t* cf) { if (this->code != nullptr) { this->code->unload_file(); } this->code = cf; }
    void delete_fileloader() { if (this->code != nullptr) { this->code->unload_file(); } }

    void run_file();
    void exit(int ec, const char* es);
    void print_exit_message();

    virtual int externals(const char* instruction = nullptr) {
        if (instruction == nullptr) return NOT_DEFINED;
        return NOT_DEFINED;
    }

    int clean_heap();

    struct mem_usage {
        size_t var_size;
        size_t func_size;
    };

    struct mem_usage get_approximate_heap_usage();

    int run_script_mode();
};

#endif