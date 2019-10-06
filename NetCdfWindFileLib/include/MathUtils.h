#pragma once
#include <vector>

size_t ProductOfElements(std::vector<size_t> sizes)
{
    size_t product = 1;

    for (size_t dim : sizes)
    {
        product *= dim;
    }

    return product;
}

