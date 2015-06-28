#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <stdexcept>
#include <cstdio>
#include <algorithm>
#include <iterator>

template<typename T>
class vector {
    T *start, *current, *ending;
private:
    void resize(bool = true);
public:
    explicit vector(int = 4);
    vector(const vector &o);
    vector& operator=(const vector &);
    ~vector();

    size_t size() const { return (current - start + 1); }
    size_t capacity() const { return (ending - start - 1); }

    void push_back(const T&);
    void pop_back();
    void remove(size_t);
    void insert(const T&, size_t);
    ssize_t find(T&) noexcept;

    void sort(bool (*comp)(const T&, const T& ) = NULL);

    T& operator[](size_t);

    //don't know how correct this is
    class iterator: public std::iterator<std::bidirectional_iterator_tag, T>{
        T* p;
        
    public:
        iterator(int* x=NULL) :p(x) {}
        iterator(const iterator& it) : p(it.p) {}
        iterator& operator++() { ++p; return *this;}
        iterator& operator--() { --p; return *this;}
        iterator operator++(int) {
            decltype(*this) tmp(*this);
            ++p;
            return tmp;
        }
        iterator operator--(int) {
            decltype(*this) tmp(*this);
            --p;
            return tmp;
        }
        bool operator==(const iterator& rhs) {return p==rhs.p;}
        bool operator!=(const iterator& rhs) {return p!=rhs.p;}
        T& operator*() {return *p;}
    };

    iterator begin(){ return iterator(start); }
    iterator end() { return iterator(current - 1); }
    iterator rbegin(){ return iterator(current - 1); }
    iterator rend(){ return iterator(start); }
};

#endif // VECTOR_H_INCLUDED
