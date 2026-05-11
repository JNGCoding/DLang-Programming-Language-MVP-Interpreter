#ifndef CPP_LIST_HPP_
#define CPP_LIST_HPP_

#include <iostream>
#include "base.hpp"

using namespace std;

template <typename Type>
class list : 
    public Collection<Type>,
    public Printable,
    public Assignable<list<Type>>,
    public Equatable<list<Type>>,
    public Slicable<Type>
{
protected:
    class node {
    public:
        node* bottom = nullptr, *top = nullptr;
        Type value;
    };

    node* tail = nullptr, *head = nullptr;
public:
    list(const size_t maxSize) : Collection<Type>(maxSize) {}
    list(const list<Type>& anotherList) : Collection<Type>( anotherList.maxSize ) {
        for (size_t i = 0; i < anotherList.count; i++) {
            this->append( anotherList.get(i) );
        }
    }

    Type* get(size_t index) {
        if (index >= this->count) return nullptr;
        node* head = this->head;
        this->head = this->tail;
        
        for (size_t i = 0; i < index; i++) {
            this->head = this->head->top;
        }

        Type* result = &this->head->value;
        this->head = head;

        return result;
    }

    const Type* get(size_t index) const {
        if (index >= this->count || index < 0) return nullptr;
        node* head = this->head;
        this->head = this->tail;
        
        for (int i = 0; i < index; i++) {
            this->head = this->head->top;
        }

        const Type* result = &this->head->value;
        this->head = head;

        return result;
    }

    void set(const Type& value, size_t index) {
        if (index >= this->count) return;
        node* head = this->head;
        this->head = this->tail;
        
        for (size_t i = 0; i < index; i++) {
            this->head = this->head->top;
        }

        this->head->value = value;
        this->head = head;
    }

    bool push(const Type& element) override {
        if (this->count >= this->maxSize) return false;

        if (this->head == nullptr) {
            this->head = new node;

            if (!this->head) {
                this->head = nullptr;
                return false;
            }

            this->head->value = element;

            this->tail = this->head;

            this->count++;
        } else {
            node* n = new node;

            if (!n) {
                n = nullptr;
                return false;
            }

            n->value = element;

            this->head->top = n;
            n->bottom = this->head;

            this->head = n;

            this->count++;
        }

        return true;
    }

    void reserve(size_t length, const Type& defaultValue) {
        while (this->count < length && this->count < this->maxSize) {
            this->push( defaultValue );
        }
    }

    bool pop(size_t index) {
        if (this->count == 0 || index >= this->count) return false;

        if (index == 0 && this->count == 1) {
            delete this->head;
            this->head = this->tail = nullptr;
            this->count--;
            return true;
        }

        node* head = this->head;
        this->head = this->tail;

        for (size_t i = 0; i < index; i++) {
            this->head = this->head->top;
        }

        node* previousNode = this->head->bottom;
        node* deletingNode = this->head;

        if (previousNode == nullptr) {
            this->tail = deletingNode->top;
            delete this->head;

            return true;
        }

        previousNode->top = deletingNode->top;

        if (deletingNode->top == nullptr) {
            head = previousNode;
        }

        delete deletingNode;
        this->head = head;

        this->count--;

        if (this->count == 0) this->tail = nullptr;        

        return true;
    }

    Type* pop() {
        if (this->count == 0) return nullptr;

        if (this->head == nullptr) {
            this->count = 0; 
            this->tail = nullptr;
            return nullptr;
        }

        node* nodeToDelete = this->head;
        Type* result = &nodeToDelete->value;

        if (this->count == 1 || this->head == this->tail) {
            this->head = nullptr;
            this->tail = nullptr;
        } else {
            this->head = nodeToDelete->bottom;
            if (this->head != nullptr) {
                this->head->top = nullptr;
            }
        }

        delete nodeToDelete;
        
        if (this->count > 0) {
            this->count--;
        }

        return result;
    }

    bool clear() {
        while (this->count > 0) {
            this->pop();
        }

        return true;
    }
    
    void print() override {
        if (this->count <= 0) { cout << "list<>()" << endl; return; }
        this->head = this->tail;
        cout << "list<>(";
        while (this->head->top) {
            cout << this->head->value << ", ";
            this->head = this->head->top;
        } cout << this->head->value << ")" << endl;
    }

    /**
     * @warning Very unsafe, must know your indices.
     */
    Type& operator[](const size_t index) override {
        node* head = this->head;
        this->head = this->tail;
        
        for (size_t i = 0; i < index && i < this->count; i++) {
            this->head = this->head->top;
        }

        Type& result = this->head->value;
        this->head = head;

        return result;
    }

    bool operator==(list<Type>& anotherList) override {
        if (this->getSize() != anotherList.getSize()) return false;

        for (size_t i = 0; i < this->getSize(); i++) {
            if (this->get(i) != anotherList.get(i)) return false;
        }

        return true;
    }

    list<Type>& operator=(list<Type>& anotherList) override {
        if (*this == anotherList) { return *this; }
        this->clear();

        for (size_t i = 0; i < anotherList.count; i++) {
            this->push( (Type) *anotherList.get(i) );
        }

        return *this;
    }
};

#endif