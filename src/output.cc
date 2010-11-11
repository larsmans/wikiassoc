/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "wikiassoc.hpp"
#include "matrix.hpp"

namespace {
    /* Reverse comparison by second member */
    template <typename T, typename U>
    inline bool lt_snd_rev(std::pair<T,U> const &x, std::pair<T,U> const &y)
    {
        return x.second > y.second;
    }
}

/**
 * Write at most n_out term associations, sorted by relevance (pf-ibf score)
 * to std::cout.
 *
 * If weights == true, output scores as well.
 */
void Matrix::output(std::size_t n_out, bool weights,
                    std::vector<std::string> const &titles) const
{
    int i, n = nrows();

    #pragma omp parallel for
    for (i=0; i<n; i++) {
        std::vector<std::pair<unsigned, Real> > related(n_out);
        related.erase(
                std::partial_sort_copy(rows[i].begin(),
                                       rows[i].end(),
                                       related.begin(), 
                                       related.end(),
                                       lt_snd_rev<unsigned, Real>),
                related.end()
            );

        std::stringstream s;
        s << titles[i] << "\n";
        for (size_t j=0; j<related.size(); j++) {
            s << "    " << titles[related[j].first];
            if (weights)
                s << " " << related[j].second;
            s << "\n";
        }

        #pragma omp critical
        std::cout << s.rdbuf();
    }
}
