/*
 * Copyright 2010-2011 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "wikiassoc.hpp"

#include "article.hpp"
#include "ibf.hpp"
#include "matrix.hpp"

namespace {
    void usage(char const *progname)
    {
        std::cerr << "usage: " << progname
                  << " [-e RE] [-n N] [-qw] pagedump linkdump\n"
                  << "    -e RE  exclude titles matching RE in output\n"
                  << "    -n N   output N associations per term, default 10\n"
                  << "    -q     quiet; no log output to standard error\n"
                  << "    -w     output pf-ibf weights with associations\n"
        ;
        std::exit(1);
    }
}

int main(int argc, char *argv[])
{
    bool output_weights = false;
    boost::regex exclude("^$");
    std::size_t n_out = 10;    // number of associations per term to output

    try {
        for (int opt; (opt = getopt(argc, argv, "e:n:qw")) != -1; ) {
            switch (opt) {
              case 'e':
                exclude = optarg;
                break;
              case 'n':
                try {
                    n_out = boost::lexical_cast<std::size_t>(optarg);
                } catch (boost::bad_lexical_cast const &e) {
                    usage(argv[0]);
                }
                break;
              case 'q':
                quiet = true;
                break;
              case 'w':
                output_weights = true;
                break;
              default:
                usage(argv[0]);
            }
        }
        argc -= optind;
        if (argc != 2)
            usage(argv[0]);
        argv += optind;
    } catch (boost::regex_error const &e) {
        // Boost.Regex error messages tend to be descriptive enough
        std::cerr << argv[0] << ": error: " << e.what() << std::endl;
        return 1;
    }

    try {
        // fail early if input files not readable
        boost::scoped_ptr<std::istream> pagefile(open_input(argv[0])),
                                        linkfile(open_input(argv[1]));

        ArticleSet articles;

        parse_pagetable(*pagefile, articles);
        pagefile.reset();

        Matrix a(articles.size()),
               r(articles.size());
        std::vector<unsigned> incoming(articles.size());
        parse_linktable(*linkfile, articles, a, incoming);
        linkfile.reset();

        logmsg("applying ibf transformation");
        InverseBacklinkFrequency ibf(incoming);
        a.transform(ibf);

        logmsg("squaring matrix");
        a.square(r);

        logmsg("computing full pf-ibf");
        // clear diagonal to avoid associating terms with themselves in output
        r.clear_diag();
        r += a;
        r.transform(normalize<2>);

        logmsg("writing output");
        r.output(n_out, output_weights, exclude, articles);

        logmsg("done");
    } catch (std::bad_alloc const &e) {
        logmsg("FATAL: out of memory");
        return 1;
    } catch (std::exception const &e) {
        logmsg(std::string("FATAL: ") + e.what());
        return 1;
    }
}
