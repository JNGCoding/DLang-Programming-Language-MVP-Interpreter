#include "code_buffer.hpp"

void codebuffer_t::read_line() {
    if (this->file != nullptr && !this->file_eof) {
        unsigned int i = 0;
        while (i < MAX_STRING_SPACE - 1) {
            int c = file->read();
            if (c == '\n') break;
            if (c == -1 || c == '\0') { this->file_eof = true; break; }
            buffer[i++] = static_cast<char>(c);
        }

        this->buffer[i] = '\0';
        this->line_index++;
    } else {
        strcpy(buffer, "No Codefile loaded");
    }
}