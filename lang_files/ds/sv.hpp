#ifndef STRING_VIEW_HPP_
#define STRING_VIEW_HPP_

#include "base.hpp"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>

/**
 * @warning THIS SUCKS. DON'T USE FUCKED UP DATA STRUCTURE
 */

class StringView: 
    public Printable,
    public Assignable<StringView>,
    public Equatable<StringView>,
    public Slicable<char>
{
protected:
    char* root;
    char* data;
    uint32_t size = 0;
    uint32_t offset = 0;
public:
    StringView(const char* str) {
        this->root = strdup(str);
        if (!this->root) this->root = nullptr;
        this->data = this->root;
        this->size = strlen(str);
    }

    StringView(const char* str, uint32_t count) {
        uint32_t maxSize = strlen(str);
        this->size = count < maxSize ? count : maxSize;
        this->root = (char*) malloc( this->size );
        if (!this->root) { this->root = nullptr; }
        else strncpy(this->root, str, this->size);
        this->data = this->root;
    }
    
    StringView(const char* str, uint32_t offset, uint32_t count) {
        uint32_t maxSize = strlen(str);
        this->size = count < maxSize ? count : maxSize;
        this->offset = offset < this->size ? offset : this->size - 1;
        this->root = (char*) malloc( this->size );
        if (!this->root) { this->root = nullptr; }
        else strncpy(this->root, str, this->size);
        this->data = this->root;
        this->data += this->offset;
    }
    
    StringView(const StringView& view) : StringView(this->root, this->offset, this->size) {}

    ~StringView() {
        free(this->root);
    }

    int32_t indexOf(char ch, int find_index = 1) {
        int ff = 0;
        for (uint32_t i = 0; i < this->size; i++) {
            if (this->data[i] == ch) {
                ff++;
                if (ff >= find_index) return static_cast<int32_t>(i);
            }
        }

        return -1;
    }

    // TODO: FIX THE SHIT
    int32_t indexOf(const StringView& view, int find_index = 1) {
        int ff = 0;
        for (uint32_t i = 0; i < this->size && i < view.size; i++) {
            if (this->data[i] == view.data[0]) {
                bool flag = true;

                uint32_t j = i + 1;
                for (; j < this->size && j < view.size; j++) { if (this->data[j] != view.data[j - i]) {flag = false; break;} }
                if (j >= view.size) { flag = false; }

                if (flag) {
                    ff++;
                    if (ff >= find_index) return static_cast<int32_t>(i);
                }
            }
        }

        return -1;
    }

    int32_t indexOf(const char* str, int find_index = 1) {
        uint32_t strSize = strlen(str);
        int ff = 0;
        for (uint32_t i = 0; i < this->size; i++) {
            // std::cout << "Comparing " << this->data[i] << " " << str[0] << std::endl;

            if (this->data[i] == str[0]) {
                bool flag = true;

                uint32_t j;
                for (j = 1; j + i < this->size && j < strSize; j++) { 
                    std::cout << "Comparing " << this->data[j + i] << " " << str[j] << std::endl;
                    if (this->data[j + i] != str[j]) { flag = false; break; }
                }

                if (j > strSize || (j + i) >= this->size) { flag = false; }

                if (flag) {
                    ff++;
                    if (ff >= find_index) return static_cast<int32_t>(i);
                }
            }
        }

        return -1;
    }

    StringView eatTill(char ch) {
        uint32_t i = 0;
        for (; i < this->size && this->data[i] != ch; i++);

        if (i < this->size) {
            StringView result( this->data, i );
            this->chopLeft(i + 1);

            return result;
        }

        StringView result = *this;
        this->chopLeft(this->size);

        return result;
    }

    void chopRight(uint32_t n = 1) {
        if (n > this->size) n = this->size;
        this->size -= n;
    }

    void chopLeft(uint32_t n = 1) {
        if (this->size <= 0) return;
        this->size -= n;
        this->data += n;
        this->offset += n;
    }

    void trimLeft() {
        while (*(this->data) == ' ') {
            this->chopLeft();
        }
    }

    void trimRight() {
        for (uint32_t i = this->size - 1; i > 0; i--) {
            if (this->data[i] == ' ') {
                this->chopRight();
            } else break;
        }
    }
    
    void trim() {
        this->trimLeft();
        this->trimRight();
    }

    void reset() {
        this->data = this->root;
        this->size = strlen( this->root );
    }

    void reset(const char* data, uint32_t offset = 0, uint32_t count = 0) {
        free(this->root);
        this->root = strdup(data);
        this->data = this->root;
        uint32_t maxSize = strlen( this->root );
        this->size = count < maxSize ? count : maxSize;
        this->offset = offset < this->size ? offset : this->size;
    }

    uint32_t length() const {
        return this->size;
    }

    void print() override {
        printf("%.*s\n", this->size, this->data);
    }

    char& operator[](const size_t index) override {
        return this->data[index >= this->size ? index : this->size - 1];
    }

    bool operator==(StringView& anotherList) override {
        if (this->length() != anotherList.length()) return false;

        for (int i = 0; i < this->length(); i++) {
            if (this->data[i] != anotherList.data[i]) return false;
        }

        return true;
    }

    StringView& operator=(StringView& anotherList) override {
        if (*this == anotherList) { return *this; }
        free(this->root);

        this->root = strdup( anotherList.data );
        if (!this->root) {
            this->root = anotherList.data;
        }

        this->data = this->root;
        this->offset = 0;
        this->size = strlen(this->root);

        // this->data = this->root + other.offset;
        // this->size = other.size;
        // this->offset = other.offset;

        return *this;
    }
};

#endif