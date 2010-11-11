/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <string>
#include "wikiassoc.hpp"

/**
 * Remove extraneous backslashes from SQL quoting.
 */
void sql_unescape(std::string &s)
{
    size_t r, w;
    for (r = w = 0; r < s.size(); r++, w++) {
        switch (s[r]) {
          case '\\':
            s[w] = s[++r];
            break;
          default:
            s[w] = s[r];
            break;
        }
    }
    s.resize(w);
}
