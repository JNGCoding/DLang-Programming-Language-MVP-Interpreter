#ifndef LANGFILES_TOKENIZER_WORD_FACTORY_HPP__
#define LANGFILES_TOKENIZER_WORD_FACTORY_HPP__

#include <stdio.h>
#include <string.h>
#include "../settings.hpp"

extern inline bool is_space(const char ch);

class word_factory_t {
protected:

public:
    char buffer[LINE_BUFFER_SIZE]   = {0};
    char next_word[MAX_STRING_SPACE] = {0};
    bool eol                         = false;
    unsigned int append_index        = 0;
    unsigned int eat_index           = 0;
    unsigned int string_size         = 0;

    word_factory_t() {}
    word_factory_t(const word_factory_t& wf) { this->copy_internals(wf); }

    void copy_internals(const word_factory_t& wf) {
        strcpy(this->buffer, wf.buffer);
        strcpy(this->next_word, wf.next_word);
        this->eol = wf.eol;
        this->append_index = wf.append_index;
        this->eat_index = wf.eat_index;
        this->string_size = wf.string_size;
    }    

    ~word_factory_t() {}

    void load_line_in_buffer(const char* str);
    char* get_next_word();

    bool end_of_line() { return this->eol; }
};

#endif