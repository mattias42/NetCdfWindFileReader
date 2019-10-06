#pragma once
#include <vector>

template<class T>
T ProductOfElements(std::vector<T> values)
{
    T product = 1;

    for (T dim : values)
    {
        product *= dim;
    }

    return product;
}

// Adds one value to each element in the vector
template<class T>
void Add(std::vector<T>& elements, T value)
{
    for (size_t ii = 0; ii < elements.size(); ++ii)
    {
        elements[ii] += value;
    }
}


