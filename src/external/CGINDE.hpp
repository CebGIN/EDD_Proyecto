#ifndef CGINDE_HPP
#define CGINDE_HPP
#include <stdexcept>
namespace cde{
template<typename T> class LinkedList;

// --- ListElement Class ---
template<typename T>
class ListElement {
    friend class LinkedList<T>;

private:
    // Pointer to the list that owns this element, required for remove_and_delete
    LinkedList<T>* list = nullptr; 

    ListElement<T>* prev = nullptr;
    T value;
    ListElement<T>* next = nullptr;

    ListElement(T tvalue, LinkedList<T>* lList, ListElement<T>* previous = nullptr, ListElement<T>* next_ptr = nullptr)
        : list(lList), prev(previous), value(tvalue), next(next_ptr) {}

public:
    T get() const {
        return value;
    }
    void set(T val) {
        value = val;
    }
    
    // Removes the element from its parent list and deletes itself
    void remove_and_delete() {
        if (list) {
            list->remove_by_LE(this);
        }
    }
};

// --- LinkedList Class ---
template<typename T>
class LinkedList {
    friend class ListElement<T>;
    
private:
    size_t size = 0;
    ListElement<T>* start = nullptr;
    ListElement<T>* end = nullptr;

    // Helper to get element at index (private utility)
    ListElement<T>* getListElement(int idx) const { 
        if (idx < 0 || idx >= size) {
            std::__throw_out_of_range("Out of range index");
            return nullptr;
        }
        
        // Simple linear search from start
        ListElement<T>* current = start;
        for (int i = 0; i < idx; ++i){
            current = current->next;
        }
        return current;
    }

public:
    LinkedList() : size(0), start(nullptr), end(nullptr) {}

    ~LinkedList() {
        clear();
    }

    int get_size() const {
        return size;
    }
    
    T get(int idx) const { 
        ListElement<T>* element = getListElement(idx);
        return element->get();
    }

    ListElement<T>* get_start() const { 
        return start;
    }

    ListElement<T>* get_end() const { 
        return end;
    }

    void remove_by_LE(ListElement<T>* elementPTR){
        if (elementPTR == nullptr) return;

        // Update list boundaries (start/end)
        if (elementPTR == start) {
            start = elementPTR->next;
        }
        if (elementPTR == end) {
            end = elementPTR->prev;
        }

        // Link previous and next elements
        ListElement<T>* behind = elementPTR->prev;
        ListElement<T>* next_node = elementPTR->next;
        
        if(behind != nullptr) behind->next = next_node;
        if(next_node != nullptr) next_node->prev = behind;

        delete elementPTR;
        --size;
    }

    void remove_at(int idx){
        if (idx < 0 || idx >= size) throw std::out_of_range("Intento eliimnar fuera de rango");

        ListElement<T>* element = getListElement(idx);
        remove_by_LE(element); // Delegate removal logic to the robust private method
    }

    void remove_by_value(const T& value){
        ListElement<T>* current = start;
        while (current != nullptr) {
            if (current->value == value) {
                remove_by_LE(current);
                return; // Found and removed the first instance, exit function
            }
            current = current->next;
        }
    }

    void push_back(const T& value) {
        ListElement<T>* newElement = new ListElement<T>(value, this, end); 

        if (start == nullptr) {
            start = newElement;
            end = newElement;
        } 
        else {
            end->next = newElement; 
            end = newElement; 
        }
        size++;
    }

    void clear() {
        ListElement<T>* current = start;
        ListElement<T>* next_node = nullptr;
    
        while (current != nullptr) {
            next_node = current->next;
            delete current;
            current = next_node;
        }
        
        start = nullptr;
        end = nullptr;
        size = 0;
    }
    
    bool go_through(ListElement<T>* &current_ptr) {
        if (current_ptr == nullptr) {
            // Start of iteration: point to the beginning of the list
            current_ptr = start;
        } else {
            // Advance to the next element
            current_ptr = current_ptr->next;
        }

        // Return true as long as the current pointer is not past the end (nullptr)
        return current_ptr != nullptr;
    }
};

// template<typename T>
// class FreeArray{
//     size_t capacity = 0;
//     T* values = nullptr;
//     size_t size = 0;

//     public: 

//     FreeArray(size_t initialSize, size_T initialCapacity = 0) : size(initialSize) {
//         capacity = std::min(initialCapacity, initialSize*2)
        
//     };

//     // FreeArray(T[] initialValues, size_T initialCapacity = 0) : capacity(initialCapacity) {
//     // }

    

// };

}
#endif