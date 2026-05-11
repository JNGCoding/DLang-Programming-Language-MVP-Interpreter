#ifndef BASIC_DEFINITIONS_HPP_
#define BASIC_DEFINITIONS_HPP_

#include <stdint.h>
#include <ostream>

#define UNSIGNED_MAX static_cast<size_t>(-1)

class Printable {
public:
    virtual void print() = 0;
};

template<typename T>
class Equatable {
public:
    virtual bool operator==(T& another) = 0;
};

template<typename T>
class Assignable {
public:
    virtual T& operator=(T& another) = 0;
};

template<typename Type>
class Slicable {
public:
    virtual Type& operator[](const size_t index) = 0;
};


// Its a placeholder
template<typename Key, typename Value>
class Pair {
private:
    Key key;
    Value val;

public:
    Pair() {}
    Pair(Key k, Value v) : key(k), val(v) {}
    Pair(const Pair<Key, Value>& pair) { this->key = pair.key; this->val = pair.val; }

    Key& getKey() {
        return this->key;
    }
    
    void setKey(const Key& k) {
        this->key = k;
    }

    Value& getValue() {
        return this->val;
    }

    void setValue(const Value& val) {
        this->val = val;
    }

    friend std::ostream& operator<<(std::ostream& os, Pair<Key, Value>& b) {
        os << "Pair<Key, Value>(" << b.getKey() << ", " << b.getValue() << ")";
        return os;
    }

    bool operator==(Pair& other) {
        return this->key == other.key && this->val && other.val;
    }

    Pair& operator=(Pair& other) {
        if (this != &other) {
            this->key = other.key;
            this->val = other.val;
        }
        return *this;
    }

    Pair& operator=(const Pair& other) {
        if (this != &other) {
            this->key = other.key;
            this->val = other.val;
        }
        return *this;
    }
};

template <typename Type>
class Collection {
protected:
size_t count = 0;
const size_t maxSize;
public:
    Collection(const size_t s) : maxSize(s) {}
    virtual ~Collection() = default;
    
    virtual Type* get(size_t index) = 0;
    virtual void set(const Type& value, size_t index) = 0;
    virtual bool push(const Type& element) = 0;
    virtual Type* pop() = 0;
    virtual bool pop(size_t index) = 0;
    virtual bool clear() = 0;

    bool isEmpty() { return this->count <= 0; }
    bool isFull() { return this->count >= this->maxSize; }
    size_t getSize() { return this->count; }
    size_t getMaxSize() { return this->maxSize; }
};

#endif