#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <stdexcept>
#include <cstdio>

template<typename T>
class vector {
    T* m_arr;
    int m_size;
    int m_capacity;

    void copy(const vector &o);
    void destroy();
    void resize();
public:
    vector(int cap=4);
    vector(const vector &o);
    vector& operator= (const vector &o);
    ~vector();

    int size() const { return m_size;}
    int capacity() const {return m_capacity;}

    void push_back(const T&);
    int pop_back();
    T& operator[](int);





};


#endif // VECTOR_H_INCLUDED
