#ifndef VECTOR_CPP
#define VECTOR_CPP

#include "vector.h"
#include <climits>


template<typename T>
vector<T>::vector(int cap) : m_size(0), m_capacity(cap)
{
    m_arr=new T[cap];
}

template<typename T>
vector<T>::vector(const vector &o)
{
    copy(o);
}

template<typename T>
vector<T>& vector<T>::operator=(const vector &o)
{
    if(this !=&o){
        copy(o);
    }
    return *this;
}

template<typename T>
vector<T>::~vector()
{
    destroy();
}


template<typename T>
void vector<T>::copy(const vector &o)
{
    m_arr=new T[o.m_capacity];
    m_size=o.m_size;
    m_capacity=o.m_capacity;

    for(size_t i=0; i<m_size; i++)
        m_arr[i]=o.m_arr[i];
}

template<typename T>
void vector<T>::destroy()
{
    delete[] m_arr;
}

template<typename T>
void vector<T>::resize(bool grow )
{
    int new_cap;
    if(grow)
        new_cap=2*m_capacity;
    else
        new_cap = m_capacity/2;

    T* tmp=m_arr;
    m_arr=new T[new_cap];

    for(size_t i=0; i<m_size; i++)
        m_arr[i]=tmp[i];
        
    delete[] tmp;
    m_capacity=new_cap;

}


template<typename T>
void vector<T>::push_back(const T& a)
{
    if(m_size==m_capacity)
        resize();
    m_arr[m_size++]=a;
}

template<typename T>
T& vector<T>::operator[](int i)
{
    if(i>=0 && i<m_size)
        return m_arr[i];
    throw std::out_of_range("blah");
}


template<typename T>
void vector<T>::pop_back()
{
   m_size--;
   if(m_size <= m_capacity/4)
       resize(false);
}

template<typename T>
void vector<T>::insert(const T& elem, int idx)
{
    if(idx < 0 || idx >= m_size)
        throw std::out_of_range("blah");

    if(m_size == m_capacity)
        resize();

    for(size_t i = idx; i < m_size-1; i++)
        m_arr[i+1] = m_arr[i];
    m_arr[idx] = elem;
    m_size++;
}

template<typename T>
void vector<T>::remove(int idx)
{
    if(idx < 0 || idx >= m_size)
        throw std::out_of_range("blah");

    for(size_t i = idx +1; i < m_size; i++)
        m_arr[i-1] = m_arr[i];
    m_size--;

    if(m_size <= m_capacity/4)
        resize(false);
}

template<typename T>
int vector<T>::find(T& elem)
{
    for(size_t i = 0; i < m_size; i++){
        if(m_arr[i] == elem)
            return i;
    }
    return -1;
}

template<typename T>
void vector<T>::sort(bool (*comp)(const T&, const T&))
{
    if(comp != NULL)
        std::stable_sort(m_arr, m_arr + m_size, comp);
    else
        std::stable_sort(m_arr, m_arr + m_size);
}

#endif

