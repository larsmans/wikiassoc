/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef _MATRIX_HPP
#define _MATRIX_HPP

#include <boost/unordered_map.hpp>
#include <algorithm>
#include <cassert>
#include <iosfwd>
#include <utility>
#include <vector>

/**
 * Square sparse matrices.
 *
 * TODO: reorder pf-ibf computations to allow for symmetric matrices
 * (see operator()) and cut memory use by half.
 */
class Matrix {
    typedef boost::unordered_map<unsigned, Real> row_type;
    std::vector<row_type> rows;

  public:
    Matrix(unsigned nr) : rows(nr) { }

    /**
     * Returns the element at row i, column j.
     * 0 if no element present.
     */
    Real operator()(unsigned i, unsigned j) const
    {
//      if (i < j)
//          std::swap(i,j);
        row_type::const_iterator rij = rows[i].find(j);
        return (rij == rows[i].cend()) ? 0. : rij->second;
    }

    /**
     * Returns a reference to the element at row i, column j.
     * Stores 0 if no element present.
     */
    Real &operator()(unsigned i, unsigned j)
    {
//      if (i < j)
//          std::swap(i,j);
        return rows[i][j];
    }

    /**
     * Add the smaller matrix other to *this. This function takes a shortcut
     * by assuming that if other(i,j) is non-zero, then so is (*this)(i,j)
     * (which is true when adding a matrix to its square).
     */
    Matrix &operator+=(Matrix const &other)
    {
        int i, n = nrows();

        #pragma omp parallel for
        for (i=0; i<n; i++) {
            row_type &row_i = rows[i];
            for (row_type::const_iterator ij  = other.rows[i].begin(),
                                          end = other.rows[i].end();
                 ij != end; ++ij) {
                unsigned j = ij->first;
                row_i[j] += ij->second;
            }
        }
        return *this;
    }

    /**
     * Clear the matrix, resetting all values to 0.
     */
    void clear()
    {
        int i, n = nrows();

        #pragma omp parallel for
        for (i=0; i<n; i++)
            rows[i].clear();
    }

    void clear_diag()
    {
        int i, n = nrows();

        #pragma omp parallel for
        for (i=0; i<n; i++)
            rows[i].erase(i);
    }

    void output(std::size_t, bool, std::vector<std::string> const &) const;

    /**
     * Apply transformation (function/functional) op to all non-zero elements
     */
    template <typename F>
    void transform(F const &op)
    {
        int i, n = nrows();

        #pragma omp parallel for
        for (i=0; i<n; ++i)
            for (row_type::iterator ij=rows[i].begin(),
                                    end=rows[i].end();
                 ij != end; ++ij)
                ij->second = op(ij->first, ij->second);
    }

    size_t nrows() const { return rows.size(); }

    /**
     * Square this matrix, storing the result in r.
     * r must be empty (all zero).
     */
    void square(Matrix &r) const { mult(*this, *this, r); }

  private:
    static void mult(Matrix const &a, Matrix const &b, Matrix &r) throw()
    {
        int i, n = a.nrows();

        #pragma omp parallel for
        for (i=0; i<n; i++) {
            // Loop over only those a(i,k) and b(k,j) that are actually stored.
            row_type const &ai = a.rows[i];
            row_type       &ri = r.rows[i];
            for (row_type::const_iterator aik = ai.begin(), ai_end = ai.end();
                 aik != ai_end; ++aik) {
                int k = aik->first;
                row_type const &bk = b.rows[k];
                for (row_type::const_iterator bkj = bk.begin(), bk_end = bk.end();
                     bkj != bk_end; ++bkj) {
                    int j = bkj->first;
                    ri[j] += aik->second * bkj->second;
                }
            }
        }
    }
};

#endif // _MATRIX_HPP
