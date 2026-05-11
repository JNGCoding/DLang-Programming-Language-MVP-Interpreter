#ifndef HASHTABLE_HPP_
#define HASHTABLE_HPP_

#include "base.hpp"
#include "list.hpp"

static size_t FNV_1a(const char* str) {
    size_t hash = 1469598103934665603ULL;
    for (int i = 0; str[i] != '\0'; i++) {
        hash ^= static_cast<uint8_t>(str[i]);
        hash *= 1099511628211ULL;
    }
    return hash;
}

template <typename T>
size_t hashing_function(T value) {
    return static_cast<size_t>(value);
}

template<>
inline size_t hashing_function<size_t>(const size_t value) {
    return value;
}

// FNV - 1a hashing algorithm
template<>
inline size_t hashing_function<const char*>(const char* value) {
    return FNV_1a(value);
}
template<>
inline size_t hashing_function<char*>(char* value) {
    return FNV_1a(value);
}

template<>
inline size_t hashing_function<void*>(void* value) {
    return reinterpret_cast<size_t>(value);
}


// create a way to store infinite positions or what not
template<typename Key, typename Value>
class hashtable:
    public Printable
//     public Assignable<Hashtable<Key, Value>>,
//     public Equatable<Hashtable<Key, Value>>,
//     public Slicable<Value*>
{
protected:
    size_t nBuckets = 0, maxCapacity = 0, objectCount = 0;
    list<Pair<Key, Value>>** buckets;
    
public:
    hashtable() : nBuckets(10), maxCapacity(UNSIGNED_MAX) {
        buckets = new list<Pair<Key, Value>>*[nBuckets];

        // We are able to do this since we store objects dynamically in the heap.
        // So we can pass any unsigned value to the constructor of 'list' and not use any memory unless we actually store objects.
        for (size_t i = 0; i < nBuckets; i++) { buckets[i] = new list<Pair<Key, Value>>(UNSIGNED_MAX); }
    }

    hashtable(const size_t maxSize) : nBuckets(10), maxCapacity(maxSize) {
        buckets = new list<Pair<Key, Value>>*[nBuckets];

        // We are able to do this since we store objects dynamically in the heap.
        // So we can pass any unsigned value to the constructor of 'list' and not use any memory unless we actually store objects.
        for (size_t i = 0; i < nBuckets; i++) { buckets[i] = new list<Pair<Key, Value>>(UNSIGNED_MAX); }
    }
    
    hashtable(size_t nbucks, size_t capsize) : nBuckets(nbucks), maxCapacity(capsize) {
        buckets = new list<Pair<Key, Value>>*[nBuckets];

        // We are able to do this since we store objects dynamically in the heap.
        // So we can pass any unsigned value to the constructor of 'list' and not use any memory unless we actually store objects.
        for (size_t i = 0; i < nBuckets; i++) { buckets[i] = new list<Pair<Key, Value>>(UNSIGNED_MAX); }
    }

    ~hashtable() {
        for (size_t i = 0; i < this->nBuckets; i++) {
            this->buckets[i]->clear();
            delete this->buckets[i];
        }

        delete[] this->buckets;
    }

    Value* get(Key k) {
        size_t index = hashing_function<Key>(k) % this->nBuckets;
        list<Pair<Key, Value>>* bucket = this->buckets[index];

        for (size_t i = 0; i < bucket->getSize(); i++) {
            if (bucket->get(i)->getKey() == k) {
                return &bucket->get(i)->getValue();
            }
        }

        return nullptr;
    }

    bool set(Key k, const Value& element) {
        size_t index = hashing_function<Key>(k) % this->nBuckets;
        list<Pair<Key, Value>>* bucket = this->buckets[index];

        for (size_t i = 0; i < bucket->getSize(); i++) {
            if (bucket->get(i)->getKey() == k) {
                bucket->get(i)->setValue( element );
                return true;
            }
        }

        return false;
    }

    bool push(Key k, const Value& element) {
        if (this->get(k) != nullptr) return false;

        if (this->objectCount >= this->maxCapacity) return false;
        size_t index = hashing_function<Key>(k) % this->nBuckets;

        if (!this->buckets[index]->isEmpty()) {
            this->objectCount++;
            return this->buckets[index]->push( Pair<Key, Value>(k, element) );
        } else {
            this->objectCount++;
            return this->buckets[index]->push( Pair<Key, Value>(k, element) );
        }
    }

    bool pop(Key k) {
        size_t index = hashing_function<Key>(k) % this->nBuckets;
        list<Pair<Key, Value>>* bucket = this->buckets[index];
        for (size_t i = 0; i < bucket->getSize(); i++) {
            if (bucket->get(i)->getKey() == k) {
                bool res = bucket->pop(i);
                if (res) this->objectCount--;
                return res;
            }
        }

        return false;
    }

    bool clear() {
        for (size_t i = 0; i < this->nBuckets; i++) { this->buckets[i]->clear(); }
        return true;
    }

    void print() override {
        for (size_t i = 0; i < this->nBuckets; i++) {
            list<Pair<Key, Value>>* bucket = this->buckets[i];
            cout << i << " --- " << bucket << " ";
            bucket->print();
        }
    }

    class iterator {
    private:
        hashtable<Key, Value>* table;
        size_t bucketIndex;
        size_t elementIndex;

    public:
        iterator(hashtable<Key, Value>* ht, size_t bIndex, size_t eIndex)
            : table(ht), bucketIndex(bIndex), elementIndex(eIndex) {
            advance_to_valid();
        }

        Pair<Key, Value>& operator*() {
            return *table->buckets[bucketIndex]->get(elementIndex);
        }

        Pair<Key, Value>* operator->() {
            return table->buckets[bucketIndex]->get(elementIndex);
        }

        iterator& operator++() {
            elementIndex++;
            advance_to_valid();
            return *this;
        }

        bool operator==(const iterator& other) const {
            return table == other.table &&
                   bucketIndex == other.bucketIndex &&
                   elementIndex == other.elementIndex;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        void advance_to_valid() {
            while (bucketIndex < table->nBuckets) {
                if (elementIndex < table->buckets[bucketIndex]->getSize()) {
                    return;
                }
                bucketIndex++;
                elementIndex = 0;
            }
        }
    };

    iterator begin() { return iterator(this, 0, 0); }
    iterator end() { return iterator(this, this->nBuckets, 0); }
};

#endif