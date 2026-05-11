#ifndef LANGFILES_UTILITY_LOGGING_HPP__
#define LANGFILES_UTILITY_LOGGING_HPP__

#include "../io/input_stream.hpp"
#include "../io/output_stream.hpp"

#include <iostream>
#include <fstream>

extern std::ofstream log_file;
extern const char* log_file_path;

extern void logger_set_write_file(char* path);
extern void logger_open_file();
extern void logger_write_file(const char* s, const bool new_line = false);
extern void logger_close_file();

class logger {
private:
    const char* root;
    bool wif = false;
public:
    output_stream_t* output = nullptr;

    enum level {
        NoOutput,
        Error,
        Warning,
        Message
    } log_level = NoOutput;

    logger() : root(nullptr) {}
    logger(level le) : root(nullptr), log_level(le) {}
    logger(const char* r, level le = NoOutput) : root(r), log_level(le) {}

    void change_output_stream(output_stream_t* new_out) { this->output = new_out; }

    void write_in_file(const bool __wif) {
        this->wif = __wif;
        // if (__wif) {
        //     logger_open_file();
        // } else logger_close_file();
    }

    bool get_wif_perm() { return this->wif; }

    void change_log_level(level le) { this->log_level = le; }

    void message(const char* str);
    void warning(const char* str);
    void error(const char* str);
};

#endif