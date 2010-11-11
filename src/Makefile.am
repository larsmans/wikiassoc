AM_CXXFLAGS = $(OPENMP_CXXFLAGS)
AM_LDFLAGS  = $(OPENMP_LDFLAGS)

bin_PROGRAMS = wikiassoc
wikiassoc_SOURCES = logmsg.cc main.cc open_input.cc output.cc parse_linktable.cc parse_pagetable.cc sql_unescape.cc
wikiassoc_LDADD = -lboost_iostreams -lbz2 -lz
