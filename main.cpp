#include <iostream>
#include <fstream>
using namespace std;

#include "lang_files/tokenizer/word_factory.hpp"
#include "lang_files/utility/ram_buffered_file.hpp"
#include "lang_files/utility/default_streams.hpp"
#include "lang_files/interpreter.hpp"
#include "lang_files/settings.hpp"

#include "lang_files/ds/hashtable.hpp"

// These objects are used to handle the io and tokenizing of the code file within the interpreter
codebuffer_t file_loader;
cout_output_stream_t out;
cin_input_stream_t in;
word_factory_t tokenizer;

// Simple interpreter loaded up with core instructions. all interpreter must inherit this base class
// in order to add new functions to it
BaseInterpreter interpreter;

int main(int argv, char* argc[]) {
    const char* filename = nullptr;
    logger::level log_level = logger::level::NoOutput;
    bool log_write_file = false;
    bool write_logs = false;
    bool script_mode = false;

    if (argv == 1) {
        script_mode = true;
    } else if (argv == 2) {
        if (argc[1][0] != '-') {
            filename = argc[1];
        } else {
            if (strcmp(argc[1], "--version") == 0) {
                out.print("ALPL (Assembly Like Programming Language) interpreter, version = ");
                out.println(VERSION);
                out.println("Created by Dhruv a.k.a JNGCoding on 22-04-2026");
            }
        }
    } else {
        for (int i = 1; i < argv; i++) {
            if (strcmp(argc[i], "--version") == 0) {
                out.print("ALPL (Assembly Like Programming Language) interpreter, version = ");
                out.println(VERSION);
                out.println("Created by Dhruv a.k.a JNGCoding on 22-04-2026");
            }
            else if (strcmp(argc[i], "-f") == 0) {
                if (i + 1 < argv) {
                    filename = argc[++i];
                } else return -2;
            }
            else if (strcmp(argc[i], "--wl") == 0) {
                if (log_level == logger::level::NoOutput) log_level = logger::level::Message;
                log_write_file = true;
            }
            else if (strcmp(argc[i], "-ll") == 0) {
                if (i + 1 < argv) {
                    const char* level = argc[++i];

                    if (strcmp(level, "message") == 0) {
                        log_level = logger::level::Message;
                    } else if (strcmp(level, "warning") == 0) {
                        log_level = logger::level::Warning;
                    } else if (strcmp(level, "error") == 0) {
                        log_level = logger::level::Error;
                    }
                } else return -2;
            }
            else if (strcmp(argc[i], "--pl") == 0) {
                write_logs = true;
            }
        }
    }

    if (filename == nullptr && !script_mode) return 0;

    if (!script_mode) {
        ifstream codefile(filename);
        if (!codefile) {
            cerr << "Error: Could not open codefile.\n";
            return 1;
        }
    
        string buffer((istreambuf_iterator<char>(codefile)), istreambuf_iterator<char>());
        const char* code = buffer.c_str();
    
        // Loading the code to buffered_file_t
        buffered_file_t file(code);
    
        // Loading the file_loader with a file for the interpreter
        file_loader.load_file(&file);

        // Setting up the interpreter
        interpreter.allocate_buffers();
        interpreter.set_debug_level(log_level);
        interpreter.set_debug_write_file(log_write_file);
        interpreter.set_tokenizer(&tokenizer);
        interpreter.set_streams(&out, (write_logs ? &out : nullptr), &in);
        interpreter.set_fileloader(&file_loader);
    
        // Run the file loaded in the interpreter
        interpreter.run_file();
    
        // Printing interpreter's return message and code
        interpreter.print_exit_message();
    
        // Exiting and cleaning the memory
        interpreter.clean_heap();
        interpreter.delete_fileloader();
    }

    if (script_mode) {
        // Setting up the interpreter
        interpreter.allocate_buffers();
        interpreter.set_debug_level(log_level);
        interpreter.set_debug_write_file(log_write_file);
        interpreter.set_tokenizer(&tokenizer);
        interpreter.set_streams(&out, (write_logs ? &out : nullptr), &in);

        // Run interpreter in script mode
        interpreter.run_script_mode();
    }

    return 0;
}