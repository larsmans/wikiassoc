/*
 * Copyright 2010-2011 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <cmath>
#include <vector>

/**
 * Inverse backlink frequency (ibf) as a function object
 */
class InverseBacklinkFrequency {
    std::size_t narticles;
    std::vector<unsigned> const &incoming;

  public:
    InverseBacklinkFrequency(std::vector<unsigned> const &incoming_)
     : narticles(incoming_.size()), incoming(incoming_)
    {
    }

    Real operator()(unsigned j, Real const &x) const
    {
        return x * std::log(narticles / incoming[j]) / std::log(2.);
    }
};

/**
 * Normalization by maximum path length using reciprocal of logarithm
 * (Nakayama 2007, bottom of p. 7).
 */
template <unsigned path_length>
Real normalize(unsigned j, Real const &x)
{
    return x / path_length;
    // Equivalent to:
    //return x / (1 + log2(path_length));
}
