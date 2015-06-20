#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <stdexcept>
#include <cstdio>
#include <algorithm>
#include <iterator>

template<typename T>
class vector {
    T* m_arr;
    int m_size;
    int m_capacity;

    void copy(const vector &o);
    void destroy();
    void resize(bool=true);
    
public:
    vector(int=4);
    vector(const vector &o);
    vector& operator= (const vector &o);
    ~vector();

    int size() const { return m_size;}
    int capacity() const {return m_capacity;}

    void push_back(const T&);
    void pop_back();
    void remove(int);
    void insert(const T&, int);
    int find(T&);


    void sort(bool (*comp)(const T&, const T& )=NULL);

    T& operator[](int);
    

    //don't know how correct this is
    class iterator: std::iterator<std::bidirectional_iterator_tag, T>{
        T* p;
        
    public:
        iterator(int* x=NULL) :p(x) {}
        iterator(const iterator& it) : p(it.p) {}
        iterator& operator++() { ++p; return *this;}
        iterator& operator--() { --p; return *this;}
        bool operator==(const iterator& rhs) {return p==rhs.p;}
        bool operator!=(const iterator& rhs) {return p!=rhs.p;}
        T& operator*() {return *p;}
    };

    iterator begin(){ return iterator(&m_arr[0]);}
    iterator end() { return iterator(m_arr + m_size);}
    iterator rbegin(){ return iterator(m_arr -1);}
    iterator rend(){ return iterator(&m_arr[m_size-1]);}
    



};


#endif // VECTOR_H_INCLUDED
