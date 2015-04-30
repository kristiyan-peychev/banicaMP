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
vector<T>& vector<T>::operator= (const vector &o){
    if(this !=&o){
        copy(o);
    }
    return *this;
}

template<typename T>
vector<T>::~vector(){
    destroy();
}


template<typename T>
void vector<T>::copy(const vector &o){
    m_arr=new T[o.m_capacity];
    m_size=o.m_size;
    m_capacity=o.m_capacity;
    for(int i=0;i<m_size;i++)
        m_arr[i]=o.m_arr[i];
}

template<typename T>
void vector<T>::destroy(){
    delete[] m_arr;
}

template<typename T>
void vector<T>::resize(){
    int newCap=2*m_capacity;
    T* tmp=m_arr;
    m_arr=new T[newCap];
    for(int i=0;i<m_size;i++)
        m_arr[i]=tmp[i];
    delete[] tmp;
    m_capacity=newCap;

}

template<typename T>
void vector<T>::push_back(const T& a){
    if(m_size==m_capacity)
        resize();
    m_arr[m_size++]=a;
}

template<typename T>
T& vector<T>::operator[](int i){
    if(i>=0 && i<m_size)
        return m_arr[i];
    //should throw exception
    throw std::out_of_range("blah");
}


template<typename T>
int vector<T>::pop_back(){
    return m_arr[--m_size];
}

#endif

