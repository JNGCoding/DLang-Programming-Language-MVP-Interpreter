#include "logging.hpp"

std::ofstream log_file;
char* log_file_path = "log.txt";

void logger_set_write_file(char* path) {
    log_file_path = path;
}

void logger_open_file() {
    log_file.open(log_file_path, std::ios::out);
}

void logger_write_file(const char* s, const bool new_line) {
    if (log_file.is_open()) {
        log_file << s;

        if (new_line) log_file << '\n';
    }
}

void logger_close_file() {
    log_file.close();
}

void logger::message(const char* str) {
    if (this->log_level >= level::Message) {
        if (root == nullptr) {
            if (this->output != nullptr) {
                this->output->print("[message] = ");
                this->output->println(str);
                this->output->flush();
            }

            if (this->wif) {
                logger_write_file("[message] = ");
                logger_write_file(str, true);
            }

        } else {
            if (this->output != nullptr) {
                this->output->print(root);
                this->output->print(" [message] = ");
                this->output->println(str);
                this->output->flush();
            }

            if (this->wif) {
                logger_write_file(root);
                logger_write_file(" [message] = ");
                logger_write_file(str, true);
            }
        }
    }
}

void logger::warning(const char* str) {
    if (this->log_level >= level::Warning) {
        if (root == nullptr) {
            if (this->output != nullptr) {
                this->output->print("[warning] = ");
                this->output->println(str);
                this->output->flush();
            }

            if (this->wif) {
                logger_write_file("[warning] = ");
                logger_write_file(str, true);
            }
        } else {
            if (this->output != nullptr) {
                this->output->print(root);
                this->output->print(" [warning] = ");
                this->output->println(str);
                this->output->flush();
            }

            if (this->wif) {
                logger_write_file(root);
                logger_write_file(" [warning] = ");
                logger_write_file(str, true);
            }
        }
    }
}

void logger::error(const char* str) {
    if (this->log_level >= level::Error) {
        if (root == nullptr) {
            if (this->output != nullptr) {
                this->output->print("[error] = ");
                this->output->println(str);
                this->output->flush();
            }

            if (this->wif) {
                logger_write_file("[error] = ");
                logger_write_file(str, true);
            }
        } else {
            if (this->output != nullptr) {
                this->output->print(root);
                this->output->print(" [error] = ");
                this->output->println(str);
                this->output->flush();
            }

            if (this->wif) {
                logger_write_file(root);
                logger_write_file(" [error] = ");
                logger_write_file(str, true);
            }
        }
    }
}