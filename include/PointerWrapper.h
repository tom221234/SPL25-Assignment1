#ifndef POINTERWRAPPER_H
#define POINTERWRAPPER_H

#include <utility>
#include <iostream>

/**
 * PointerWrapper - A template class that wraps a raw pointer
 * 
 * This is Phase 3 of the assignment. Students must analyze and implement
 * a complete pointer wrapper class that manages dynamic memory safely.
 * 
 * Refer to the assignment instructions (Phase 3) for detailed guiding questions
 * about resource management, ownership semantics, copy vs move, and interface design.
 */

 /**
 * ITAMAR: - in this class we had 2 options:
 * 1) the wrapper is not the owner
 * 2) wrapper is the sole owner, like unqptr
 * 
 * We went with the 2nd approch, the first one could lead to double deletions or we will not delete at all.
 * 
 */
template<typename T>
class PointerWrapper {
private:
    T* ptr;  // Raw pointer to the managed object

public:
    // ========== CONSTRUCTION AND DESTRUCTION ==========

    /**
     * Default constructor - creates empty wrapper
     */
    PointerWrapper() : ptr(nullptr) {}

    /**
     * Constructor from raw pointer - wraps the pointer
     */
    explicit PointerWrapper(T* p) : ptr(p) {}

    /**
     * TODO: Implement destructor
     * HINT: What should happen to the wrapped pointer when the wrapper is destroyed?
     * Think about ownership and resource management.
     * Is the default destructor sufficient here? ITAMAR: i think it is, i think we can remove the if
     */
    ~PointerWrapper() {

        // delets only if the pointer is not null - why?
        if (ptr != nullptr) delete ptr;
    }

    // ========== COPY OPERATIONS (DELETED) ==========

    /**
     * Copy constructor is DELETED
     * Think about why this might be necessary for a pointer wrapper
     * ITAMAR: - Because if we dont delete this func, it could lead to double delete. maybe double pointers?
     */
    PointerWrapper(const PointerWrapper& other) = delete;

    /**
     * Copy assignment is DELETED
     * Consider what problems could arise if copying was allowed
     * ITAMAR: - it could lead to double pointers.
     */
    PointerWrapper& operator=(const PointerWrapper& other) = delete;

    // ========== MOVE OPERATIONS (STUDENTS IMPLEMENT) ==========

    /**
     * TODO: Implement move constructor
     * HINT: How should ownership transfer from one wrapper to another?
     * What should happen to the source wrapper after the move? ITAMAR: - after the move, the source wrapper should be nullptr.
     */
    PointerWrapper(PointerWrapper&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr; // Leave other in a null state
    }

    /**
     * TODO: Implement move assignment operator - COMPLETED
     * HINT: Handle cleanup of current resource and ownership transfer
     * Don't forget about self-assignment! - NOTICED
     */
    PointerWrapper& operator=(PointerWrapper&& other) noexcept {

        // self-assignment
        if (this != &other) { 
            if (ptr != nullptr) {
                delete ptr;
            }

            // move assignment operator usage
            ptr = other.ptr;
            other.ptr = nullptr;
        }

        // Return ptr
        return *this;
    }

    // ========== ACCESS OPERATIONS ==========

    /**
     * TODO: Implement dereference operator
     * EXPLENATION: This operator will return the obj itself.
     * HINT: How do you access the object that the wrapper points to? Is this operation implementation complete?
     * ITAMAR: added a check if the ptr is null.
     * @throws std::runtime_error if ptr is null
     */

    T& operator*() const {
        if (ptr == nullptr) throw std::runtime_error("Dereferencing null PointerWrapper");
        return *ptr;
    };

    /**
     * TODO: Implement arrow operator
     * EXPLENATION: This operator will return a raw pointer (raw pointer is a normal pointer) of ptr.
     * HINT: How do you access members of the wrapped object? ITAMAR: by returning a raw ptr.
     * What safety checks should you perform? ITAMAR: i need to do safety checks? what if the ptr is null? line 127.
     * ITAMAR: note: -> and get() have the implementation but they are diff cuz of cpp special syntax rules
     * ITAMAR: my i need to add a @throws here? to be consistent with the rest of the ACCESS OPERATIONS.
     */
    T* operator->() const {
        return ptr;
    }

    /**
     * TODO: Implement get() function
     * HINT: Sometimes you need access to the raw pointer without changing ownership
     * EXPLENATION: This operator is used for get() usage, it will give accsess to the obj like a ptr but will not give other info (i think the reason is the keep the encapulation).
     * What should this function return - ITAMAR: it should return the obj but not ptr to it.
     * ITAMAR: note: -> and get() have the same implementation but they are diff cuz of cpp special syntax rules.
     * @throws std::runtime_error if ptr is null
     */
    T* get() const {
        if (ptr == nullptr) throw std::runtime_error("Getting null pointer from PointerWrapper");
        return ptr;
    }

    // ========== OWNERSHIP MANAGEMENT ==========

    /**
     * TODO: Implement release() function
     * HINT: What does "release" mean in terms of ownership? ITAMAR: it means that the Wrapper no longer owns the pointer.
     * Should the wrapper still own the pointer after calling release()? ITAMAR: no, it will be nullptr.
     */
    T* release() {
        T* temp = ptr; // is this the copy cont for temp?
        ptr = nullptr; 
        return temp;
    }

    /**
     * TODO: Implement reset() function
     * HINT: How do you replace the currently wrapped pointer? ITAMAR: ill delete curr, then assign new ptr as ptr.
     * What should happen to the old pointer? ITAMAR: the old pointer needs to be deleted.
     */
    void reset(T* new_ptr = nullptr) {
        if (ptr != new_ptr) { // avoid self-reset
            if (ptr != nullptr) {
                delete ptr;  // clean up old resource
            }
            ptr = new_ptr; // save new pointer
        }
        
    }

    // ========== UTILITY FUNCTIONS ==========

    /**
     * TODO: Implement boolean conversion operator
     * HINT: When should a wrapper be considered "true" or "false"? ITAMAR: if null return false, else return true.
     * Why might the explicit keyword be important here? ITAMAR: To prevent unwanted usage of Ptrwrapper instead of rawptr - or convertion to bool.
     */
    explicit operator bool() const {
        return ptr != nullptr; // true if ptr is not null
    }

    /**
     * Swap two PointerWrapper objects
     * This is implemented for you as a reference
     */
    void swap(PointerWrapper& other) noexcept {
        std::swap(ptr, other.ptr);
    }
};

// ========== NON-MEMBER FUNCTIONS ==========

/**
 * Helper function to create PointerWrapper
 * This is implemented for you as an example
 * Can you figure out when this would be useful in phase 4? ITAMAR: This func will make init of ptrWrap obj less prone to mistakes.
 */
template<typename T, typename... Args>
PointerWrapper<T> make_pointer_wrapper(Args&&... args) {
    return PointerWrapper<T>(new T(std::forward<Args>(args)...));
}

/**
 * TODO: Implement swap for PointerWrapper
 * HINT: How can you swap two wrapper objects?
 * Why might this be useful? ITAMAR: bcz we cant copy (deleted) and bcz the defualt std::swap is using move
 */
template<typename T>
void swap(PointerWrapper<T>& lhs, PointerWrapper<T>& rhs) noexcept {
    // TODO: Implement global swap function
    // HINT: You can use the member swap function
    //your code here...
    lhs.swap(rhs);
}

#endif // POINTERWRAPPER_H