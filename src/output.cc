/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <boost/iterator/filter_iterator.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "wikiassoc.hpp"

#include "article.hpp"
#include "matrix.hpp"

namespace {
    /* Reverse comparison by second member */
    template <typename T, typename U>
    inline bool GtBySecond(std::pair<T,U> const &x, std::pair<T,U> const &y)
    {
        return x.second > y.second;
    }

    class IncludeFilter
    {
        ArticleSet const &articles;
        boost::regex const &exclude;

      public:
        IncludeFilter(boost::regex const &excl, ArticleSet const &as)
          : articles(as), exclude(excl) {}

        bool operator()(std::pair<unsigned, Real> const &iw)
        { return not boost::regex_match(articles[iw.first].title, exclude); }
    };
}

/**
 * Write at most n_out term associations, sorted by relevance (pf-ibf score)
 * to std::cout. Skips over terms that match the RE exclude.
 *
 * If weights == true, output scores as well.
 */
void Matrix::output(std::size_t n_out, bool weights,
                    boost::regex const &exclude,
                    ArticleSet const &articles) const
{
    int i, n = nrows();
    IncludeFilter include(exclude, articles);

    #pragma omp parallel for
    for (i=0; i<n; i++) {
        std::vector<std::pair<unsigned, Real> > related(n_out);

        // Filter by the RE first, so we still get n_out items if possible
        boost::filter_iterator<IncludeFilter, row_type::const_iterator>
            begin(include, rows[i].begin(), rows[i].end()),
            end(  include, rows[i].end(),   rows[i].end());

        related.erase(
                std::partial_sort_copy(begin, end,
                                       related.begin(), related.end(),
                                       GtBySecond<unsigned, Real>),
                related.end()
            );

        std::stringstream s;
        s << articles[i].title << "\n";
        for (size_t j=0; j<related.size(); j++) {
            s << "    " << articles[related[j].first].title;
            if (weights)
                s << " " << related[j].second;
            s << "\n";
        }

        #pragma omp critical
        std::cout << s.rdbuf();
    }
}
