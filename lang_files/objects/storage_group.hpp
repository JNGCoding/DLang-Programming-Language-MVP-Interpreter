#ifndef LANGFILES_OBJECTS_STORAGE_GROUP_HPP__
#define LANGFILES_OBJECTS_STORAGE_GROUP_HPP__

#include <stdint.h>
#include <stdint.h>
#include <string.h>
#include "../ds/hashtable.hpp"

template<typename T>
class storage_group_t : public hashtable<char*, T> {
public:
    storage_group_t(const size_t maxSize) : hashtable<char*, T>(maxSize) {}
    virtual ~storage_group_t() = default;

    T* get(const char* k) {
        size_t index = hashing_function<const char*>(k) % this->nBuckets;
        
        list<Pair<char*, T>>* bucket = this->buckets[index];

        for (size_t i = 0; i < bucket->getSize(); i++) {
            char* pair_key = bucket->get(i)->getKey();

            if (strcmp(pair_key, k) == 0) {
                return &bucket->get(i)->getValue();
            }
        }

        return nullptr;
    }

    bool set(const char* k, const T& element) {
        size_t index = hashing_function<const char*>(k) % this->nBuckets;
        list<Pair<char*, T>>* bucket = this->buckets[index];

        for (size_t i = 0; i < bucket->getSize(); i++) {
            if (strcmp(bucket->get(i)->getKey(), k) == 0) {
                bucket->get(i)->setValue( element );
                return true;
            }
        }

        return false;
    }
    
    bool push(const char* k, const T& element) {
        // if (this->get(k) == nullptr) {
            char* key = strdup(k);
            return hashtable<char*, T>::push(key, element);
        // }

        return false;
    }

    bool pop(const char* k) {
        size_t index = hashing_function<const char*>(k) % this->nBuckets;
        list<Pair<char*, T>>* bucket = this->buckets[index];

        for (size_t i = 0; i < bucket->getSize(); i++) {
            if (strcmp(bucket->get(i)->getKey(), k) == 0) {
                // Get the heap allocated key
                char* key = bucket->get(i)->getKey();

                // Pop the element
                bool res = bucket->pop(i);
                if (res) this->objectCount--;

                // key will not be freed, so free it now
                free(key);

                return res;
            }
        }

        return false;        
    }

    bool clear() {
        for (Pair<char*, T>& element : *this) {
            free( element.getKey() );
        }

        return hashtable<char*, T>::clear();
    }

    size_t get_count() {
        return this->objectCount;
    }
};

#endif