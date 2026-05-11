#include "space_allocator.hpp"

hashtable<allocator_block, size_t> allocations;

// Initializing basic_allocator variables
char basic_allocator::id = 'B';
size_t basic_allocator::allocated_size = 0;
size_t basic_allocator::allocation_count = 0;

namespace basic_allocator {
    void* take_space(size_t _size, bool zero_out) {
        void* res = malloc(_size);

        if (res != nullptr) {
            if (zero_out) memset(res, 0, _size);

            allocator_block block;
            block._ptr_block = res;
            block._alloc_id = basic_allocator::id;

            allocations.push(block, _size);
            
            // We will have a little bias since we also have to store the size_t variable in the our dictionary
            allocated_size += (_size + sizeof(size_t));
            allocation_count++;
        }

        return res;
    }

    void give_space(void* _ptr) {
        allocator_block block;
        block._ptr_block = _ptr;
        block._alloc_id = basic_allocator::id;

        size_t* _size = allocations.get(block);

        if (_size != nullptr) {
            allocated_size -= ((*_size) + sizeof(size_t));
            allocations.pop(block);
            allocation_count--;
        }

        free(_ptr);
    }

    size_t get_allocated_size(bool with_bias) {
        return allocated_size + (with_bias ? 0 : -(allocation_count * sizeof(size_t)));
    }
};

// Initializing pool_allocator variables
char pool_allocator::id = 'P';
void* pool_allocator::space = nullptr;

namespace pool_allocator {
    bool master_allocate(size_t _size, bool zero_out) {
        if (space != nullptr) {
            basic_allocator::give_space(space);
            space = nullptr;
        }

        space = basic_allocator::take_space(_size);
        if (zero_out) memset(space, 0, _size);

        return space != nullptr;
    }

    void* take_space(size_t _size) {
        (void) _size;
        return nullptr;
    }

    void give_space(void* _ptr) {
        (void) _ptr;
    }

    bool master_free() {
        if (space != nullptr) {
            basic_allocator::give_space(space);
            space = nullptr;
            return true;
        } else return false;
    }

    void set_master(void* _ptr) {
        space = _ptr;
    }
};

// Initializing static_allocator variables
char linear_allocator::id = 'L';
void* linear_allocator::space = nullptr;
unsigned int linear_allocator::alloc_pointer = 0;
unsigned int linear_allocator::buffer_size = 0;
unsigned int linear_allocator::latest_allocation = 0;

namespace linear_allocator {
    bool master_allocate(size_t _size, bool zero_out) {
        if (space != nullptr) {
            basic_allocator::give_space(space);
            space = nullptr;
        }

        space = basic_allocator::take_space(_size);
        if (zero_out) memset(space, 0, _size);

        buffer_size = _size;
        alloc_pointer = 0;
        
        return space != nullptr;
    }

    void* take_space(size_t _size) {
        alloc_pointer = ALIGN_SIZE(alloc_pointer);
        if (alloc_pointer + _size <= buffer_size && space != nullptr) {
            void* res = static_cast<char*>(space) + alloc_pointer;
            alloc_pointer += _size;
            latest_allocation = _size;
            return res;
        } else return nullptr;
    }

    void clear_space(bool zero_out) {
        if (zero_out) memset(space, 0, buffer_size);
        alloc_pointer = 0;
    }

    bool master_free() {
        if (space != nullptr) {
            basic_allocator::give_space(space);
            space = nullptr;
            buffer_size = 0;
            return true;
        } else return false;
    }

    void set_master(void* _ptr) {
        space = _ptr;
    }
};