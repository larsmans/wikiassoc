/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <cstring>
#include <ctime>
#include <iostream>

#include "config.h"
#include "wikiassoc.hpp"

bool quiet = false;

void logmsg(char const *msg)
{
    if (quiet)
        return;

    time_t t = std::time(0);
    char *timetxt = std::ctime(&t);         // XXX: not reentrant
    *std::strchr(timetxt, '\n') = '\0';     // ISO guarantees a '\n'

    std::clog << PACKAGE_NAME << " [" << timetxt << "]: " << msg << std::endl;
}

void logmsg(std::string const &msg) { logmsg(msg.c_str()); }
