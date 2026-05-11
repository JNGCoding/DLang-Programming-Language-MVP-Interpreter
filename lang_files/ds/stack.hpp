#ifndef STACK_HPP_
#define STACK_HPP_

#include "base.hpp"

#include <iostream>
using namespace std;

template <typename Type>
class stack : 
    public Collection<Type>,
    public Printable,
    public Assignable<stack<Type>>,
    public Equatable<stack<Type>>,
    public Slicable<Type>
{
private:
    Type* buffer;
public:
    stack(const size_t maxSize) : Collection<Type>(maxSize) {
        this->buffer = new Type[maxSize];
    }

    stack(const Type buffer[], const size_t maxSize) : Collection<Type>(maxSize) {
        this->buffer = new Type[maxSize];

        for (size_t i = 0; i < maxSize; i++) {
            this->buffer[i] = buffer[i];
            this->count++;
        }
    }

    stack(const stack<Type>& as) : Collection<Type>( as.maxSize ) {
        this->buffer = new Type[ as.maxSize ];

        for (size_t i = 0; i < as.maxSize; i++) {
            this->buffer[i] = as.buffer[i];
        }

        this->count = as.count;
    }

    ~stack() {
        delete[] this->buffer;
    }

    Type* get(size_t index) override {
        if (index >= this->count) return nullptr;
        return (this->buffer + index);
    }

    void set(const Type& value, size_t index) override {
        if (index >= this->count) return;
        this->buffer[index] = value;
    }

    bool push(const Type& element) override {
        if (this->count >= this->maxSize) return false;
        this->buffer[this->count++] = element;
        return true;
    }

    Type* pop() override {
        if (this->isEmpty()) return nullptr;
        return &this->buffer[(this->count--) - 1];
    }

    Type* peek() {
        if (this->isEmpty()) return nullptr;
        return &this->buffer[this->count - 1];
    }

    bool pop(size_t index) override {
        if (this->isEmpty() || index >= this->count) return false;

        for (size_t i = index; i < this->count - 1; i++) {
            this->buffer[i] = this->buffer[i + 1];
        }

        this->count--;
        return true;
    }

    bool clear() override {
        this->count = 0;
        return true;
    }

    void print() override {
        if (this->isEmpty()) { cout << "stack<>()" << endl; return; }

        cout << "stack<>(";
        for (size_t i = 0; i < this->count - 1; i++) {
            cout << this->buffer[i] << ", ";
        } cout << this->buffer[this->count - 1] << ")" << endl;
    }

    Type& operator[](size_t index) override {
        return this->buffer[index >= this->count ? this->count - 1 : index];
    }

    bool operator==(stack<Type>& as) override {
        if (this->getSize() != as.getSize()) return false;

        for (size_t i = 0; i < this->getSize(); i++) {
            if (this->get(i) != as.get(i)) return false;
        }

        return true;
    }

    stack<Type>& operator=(stack<Type>& as) override {
        if (*this == as) { return *this; }
        this->clear();

        for (size_t i = 0; i < as.count; i++) {
            this->push( *as.get(i) );
        }

        return *this;
    }
};

#endif