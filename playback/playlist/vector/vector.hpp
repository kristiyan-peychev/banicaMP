#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "vector.h"
#include <climits>

template<typename T>
vector<T>::vector(int cap) : start(new T[cap]), current(start - 1), 
        ending(start + cap + 1)
{ }

template<typename T>
vector<T>::vector(const vector &o)
{
    T *tmp = new T [o.size()];
    T *c = o.start, *tmpcpy = tmp;

    while (c != o.ending) {
        *tmpcpy = *c++;
        tmpcpy++;
    }

    start = tmp;
    ending = start + o.size();
    current = start + (o.current - o.start);
}

template<typename T>
vector<T> &vector<T>::operator=(const vector &o)
{
    if(this == &o)
        return *this;

    T *tmp = new T [o.size()];
    T *c = o.start, *tmpcpy = tmp;

    while (c != o.ending) {
        *tmpcpy = *c++;
        tmpcpy++;
    }

    delete[] start;

    start = tmp;
    ending = start + o.size() + 1;
    current = start + (o.current - o.start);

    return *this;
}

template<typename T>
vector<T>::~vector()
{
    delete[] start;
}

#include <iostream>
template<typename T>
void vector<T>::resize(bool grow)
{
    int new_cap;
    if(grow)
        new_cap = size() << 1;
    else
        new_cap = size() >> 1;

    T *tmp = new T[new_cap];
    T *tmpcpy = tmp, *itr = start;

    while (itr != ending) {
        *tmpcpy = *itr++;
        tmpcpy++;
    }

    std::swap(start, tmp); // FIXME
    ending = start + new_cap + 1;
    current = start + (current - tmp);
    delete[] tmp;
}

template<typename T>
void vector<T>::push_back(const T& a)
{
    if((current + 1) >= ending)
        resize();
    *++current = a;
}

template<typename T>
T& vector<T>::operator[](size_t i)
{
    std::cout << start + i << ' ' << current << '\n' << i << '\n';
    if ((start + i) < current)
        return start[i];

    throw std::out_of_range("index out of range");
}


template<typename T>
void vector<T>::pop_back()
{
    if (current < start)
        throw std::out_of_range("array is empty");

    --current;
    // I believe this should be removed >.<
    if(((size_t) start + (size_t) current) <= 
            ((size_t) ending - (size_t) start) / 4)
        resize(false);
}

template<typename T>
void vector<T>::insert(const T &elem, size_t idx)
{
    T *el = start + idx;
    if (el > current)
        throw std::out_of_range("index out of range");

    if ((current + 1) >= ending)
        resize();

    while (el < current)
        ++el = *(el - 1);

    ++current;
}

template<typename T>
void vector<T>::remove(size_t idx)
{
    T *el = start + idx, *c = current;
    if (el > current)
        throw std::out_of_range("index out of range");

    while (c > el)
        *--c = *(c + 1);

    if(((size_t) start + (size_t) current) <= 
            ((size_t) ending - (size_t) start) / 4)
        resize(false);
}

template<typename T>
ssize_t vector<T>::find(T& elem) noexcept
{
    for (T *itr = start; itr < current; itr++) 
        if(*itr == elem)
            return (itr - start);

    return static_cast<ssize_t>(-1L);
}

template<typename T>
void vector<T>::sort(std::function<bool(T&, T&)> &compar)
{
    // TODO
}

template<typename T>
bool vector<T>::empty(void) const
{
    return size() == 0;
}

template<typename T>
void vector<T>::clear(void) noexcept
{
    current = start;
}

#endif
