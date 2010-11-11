/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <fstream>

#include "wikiassoc.hpp"

namespace io = boost::iostreams;
using namespace std;

namespace {
    class InputStream : public io::filtering_istream {
        ifstream file;

      public:
        InputStream(char const *path)
         : file(path, ios_base::in | ios_base::binary)
        {
            using boost::algorithm::iends_with;

            if (iends_with(path, ".gz"))
                push(io::gzip_decompressor());
            else if (iends_with(path, ".bz2"))
                push(io::bzip2_decompressor());
            push(file);
        }
    };
}

/**
 * Open input file, return istream* with gzip or bzip2 decompressor
 * stacked in if necessary.
 */
istream *open_input(char const *path)
{
    return new InputStream(path);
}
