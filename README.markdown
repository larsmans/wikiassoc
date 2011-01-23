Wikiassoc
=========

Wikiassoc is a tool for generating term associations by analyzing the link
structure of Wikipedia (or any other wiki based on the
[MediaWiki](http://mediawiki.org) software).

To build and install Wikiassoc, you need a fairly modern C++ compiler
(tested with GCC 4.3 and Open64 4.2.2.2), the [Boost](http://www.boost.org)
libraries (specifically Boost.IOStreams and Boost.Regex),
[zlib](http://zlib.net/), [bzlib](http://www.bzip.org/) and the GNU autools
(autoconf and automake).

It is highly advisable to use

* a C++ compiler that supports OpenMP
* [google-sparsehash](http://code.google.com/p/google-sparsehash/)

as Wikiassoc will be very slow or consume huge amounts of memory without these.

Wikiassoc uses the GNU build tools. Enter

    ./prepare
    ./configure && make && make install

or see the file INSTALL for more detailed instructions.


Usage
-----

To compile an associative thesaurus with Wikiassoc, first download some files
from the [Wikimedia dump repository](http://download.wikimedia.org).
For example, say you want word associations in Latin. Fetch the files

    lawiki-YYYYMMDD-page.sql.gz
    lawiki-YYYYMMDD-pagelinks.sql.gz

and run the Wikiassoc program as

    wikiassoc lawiki-YYYYMMDD-page.sql.gz lawiki-YYYYMMDD-pagelinks.sql.gz \
      | gzip -c > lawiki-associations.gz

(Using gzip is highly recommended, as Wikiassoc produces a lot of output.)

You will get a log of what's happening on stderr. Note that Wikiassoc takes
a *lot* of memory; on the larger Wikipedias, it may be as much as 12GB or
more.

In `lawiki-associations.gz`, you will find a text file with terms and indented
associations for the term:

    Astronomia
        Scientia
        Physica
        Universum
        Galaxias
        Planeta
        Geologia
        Mathematica
        Stella
        Luna
        Terra

See the manpage for details (`man wikiassoc`).


How does it work?
-----------------

For each article in the Wikipedia database dump, Wikiassoc looks at all the
articles that can be reached by following at most two links. It then weighs
all these articles by a scheme called pf-ibf, or path frequency-inverse
backlink frequency. For further explanation, refer to:

Nakayama, K., Hara, T. and Nishio, S. (2007)
[Wikipedia Mining for an Association Web Thesaurus Construction](http://wikipedia-lab.org/en/images/9/90/Wise2007.pdf).
In Proc. International Conference on Web Information Systems Engineering
(WISE), pp. 322-334.
