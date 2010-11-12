/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef WIKITHES_HPP
#define WIKITHES_HPP

#include <boost/unordered_map.hpp>
#include <iosfwd>
#include <string>
#include <vector>

// The Wikipedia main namespace, where the encyclopedic content resides.
const int WIKIPEDIA_MAIN_NS = 0;

class Matrix;

// Type for weight calculations. Redefine as double or bigger if needed;
// float suffices for 6e5 articles.
// TODO: put this in config.h
typedef float Real;

extern bool quiet;      // disable log output to stderr

void logmsg(char const *);
void logmsg(std::string const &);
std::istream *open_input(char const *);
void parse_linktable(std::istream &, Matrix &,
                     boost::unordered_map<unsigned, unsigned> const &,
                     boost::unordered_map<std::string, unsigned> const &,
                     std::vector<unsigned> &);
void parse_pagetable(std::istream &, 
                     boost::unordered_map<unsigned, unsigned> &,
                     boost::unordered_map<std::string, unsigned> &,
                     std::vector<std::string> &);
void sql_unescape(std::string &);

#endif  // WIKITHES_HPP
