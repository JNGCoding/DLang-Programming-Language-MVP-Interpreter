#ifndef LANGFILES_SPACE_ALLOCATOR_HPP__
#define LANGFILES_SPACE_ALLOCATOR_HPP__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "ds/hashtable.hpp"

#define MAX_ALIGNMENT (alignof(long long) > alignof(double) ? alignof(long long) : alignof(double))
#define ALIGN_SIZE(x) ((x + MAX_ALIGNMENT - 1) & ~(MAX_ALIGNMENT - 1)) 

/**
 * @note Ram that is allocated in a specific allocator must be freed using the free() function of every allocator;
 */

// For storing allocations
class allocator_block {
public:
    void* _ptr_block;
    char _alloc_id;

    bool operator==(const allocator_block& other) const {
        return _ptr_block == other._ptr_block && _alloc_id == other._alloc_id;
    }

    friend std::ostream& operator<<(std::ostream& os, const allocator_block& block) {
        os << "allocator_block[" << block._ptr_block << ", " << block._alloc_id << "]";
        return os;
    }
};

// Specialization of hashing_function for allocator_block
template<>
inline size_t hashing_function<allocator_block>(allocator_block value) {
    size_t h1 = hashing_function<void*>(value._ptr_block);
    size_t h2 = hashing_function<char>(value._alloc_id);
    return h1 ^ (h2 << 1);
}

extern hashtable<allocator_block, size_t> allocations;

/**
 * @brief depends on C or C++ runtime's heap manager for allocating memory
 */
namespace basic_allocator {
    extern size_t allocated_size;
    extern size_t allocation_count;
    extern char id;

    /**
     * @brief this function will allocate a block of memory with the help of C/C++ runtime heap manager
     *        also increments the `allocated_size` counter with the bias of `sizeof(size_t)` as an indicator of storing data into
     *        the allocations hashtable.
     * 
     * @param _size size of the memory chunk
     */
    void* take_space(size_t _size, bool zero_out = false);

    /**
     * @brief this function frees any block of memory allocated with `basic_allocator`
     *        also decrements `allocated_size` counter with the bias of `sizeof(size_t)` as an indicator of removing data from
     *        the allocations hashtable
     * 
     * @param _ptr memory address of the chunk of memory
     */
    void give_space(void* _ptr);

    /**
     * @brief get the total allocated size allocated via `basic_allocator`
     * 
     * @param with_bias get the allocated size with the bias of allocations
     */
    size_t get_allocated_size(bool with_bias = false);
};

/**
 * @brief uses `basic_allocator` for allocating a large block of memory, managing objects in that chunk of memory
 */
namespace pool_allocator {
    extern void* space;
    extern char id;

    /**
     * @brief this function allocates the master `space` void* in which the allocator will allocate objects from
     * 
     * @param _size total size of the memory chunk allocated
     * 
     * @warning if `space` is already allocated, then the function will try to deallocate the `space` first in order to re-allocate fresh chunk of memory
     */
    bool master_allocate(size_t _size);

    void* take_space(size_t _size);
    void give_space(void* _ptr);

    /**
     * @brief this function frees the master `space` void*
     * 
     * @note also deallocates all the objects allocated via this allocator
     */
    bool master_free();

    /**
     * @brief this function directly sets the master `space` void* to a user allocated ptr
     *
     * @param _ptr user allocated ptr which will be copied to space
     * 
     * @warning `master_allocate` and `master_free` functions may not work properly after the use of this function
     */
    void set_master(void* _ptr);
};

namespace linear_allocator {
    extern void* space;
    extern char id;
    extern unsigned int alloc_pointer;
    extern unsigned int latest_allocation;
    extern unsigned int buffer_size;

    /**
     * @brief this function allocates the master `space` void* in which the allocator will allocate objects from
     * 
     * @param _size total size of the memory chunk allocated
     * 
     * @warning if `space` is already allocated, then the function will try to deallocate the `space` first in order to re-allocate fresh chunk of memory
     */
    bool master_allocate(size_t _size, bool zero_out = false);

    /**
     * @brief this function will allocate a block of memory from the already allocated buffer
     *        also increments the `alloc_pointer`
     * 
     * @param _size size of the memory chunk
     */
    void* take_space(size_t _size);

    /**
     * @brief this function will reset the `alloc_pointer` to 0
     * 
     * @param zero_out zero-out the buffer
     */
    void clear_space(bool zero_out = false);

    /**
     * @brief this function frees the master `space` void*
     * 
     * @note also deallocates all the objects allocated via this allocator
     */
    bool master_free();

    /**
     * @brief this function directly sets the master `space` void* to a user allocated ptr
     *
     * @param _ptr user allocated ptr which will be copied to space
     * 
     * @warning `master_allocate` and `master_free` functions may not work properly after the use of this function
     */
    void set_master(void* _ptr);
};

#endif