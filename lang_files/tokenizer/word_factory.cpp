#include "word_factory.hpp"

inline bool is_space(const char ch) {
    return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\r' || ch == '\f' || ch == '\n';
}

void word_factory_t::load_line_in_buffer(const char* str) {
    snprintf(this->buffer, MAX_STRING_SPACE, str);
    this->string_size = strlen(this->buffer);
    this->append_index = 0;
    this->eat_index = 0;
    this->next_word[0] = '\0';
    this->eol = false;

    // Skip leading spaces
    while (this->eat_index < this->string_size && is_space(this->buffer[this->eat_index])) this->eat_index++;
    if (this->eat_index >= this->string_size) this->eol = true;
}

char* word_factory_t::get_next_word() {
    this->append_index = 0;

    auto walk_till_char_end = [&](const char ch) {
        while (this->eat_index < this->string_size && this->append_index < MAX_STRING_SPACE - 1) {
            char ic = this->buffer[this->eat_index++];
            this->next_word[this->append_index++] = ic;

            if (ic == ch) {
                break;
            }
        }
    };

    while (this->eat_index < this->string_size && this->append_index < MAX_STRING_SPACE - 1) {
        char c = this->buffer[this->eat_index++];
        
        if (is_space(c)) {
            break;
        } else if (c == '"') {
            this->next_word[this->append_index++] = c;
            walk_till_char_end('"');
        } else if (c == '{') {
            this->next_word[this->append_index++] = c;

            while (this->eat_index < this->string_size && this->append_index < MAX_STRING_SPACE - 1) {
                char ic = this->buffer[this->eat_index++];

                if (ic == ',') {
                    this->next_word[this->append_index++] = '|';
                } else if (ic == '"') {
                    this->next_word[this->append_index++] = ic;
                    walk_till_char_end('"');
                } else if (!is_space(ic)) {
                    this->next_word[this->append_index++] = ic;
                }

                if (ic == '}') {
                    break;
                }
            }
        } else {
            this->next_word[this->append_index++] = c;
        }

        // else if (c == '[') {
        //     this->next_word[this->append_index++] = c;
        //     walk_till_char_end(']');
        // }
    }

    // Skip trailing spaces
    while (this->eat_index < this->string_size && is_space(this->buffer[this->eat_index])) this->eat_index++;

    this->next_word[this->append_index] = '\0';

    if (this->eat_index >= this->string_size) this->eol = true;

    return this->next_word;
}
