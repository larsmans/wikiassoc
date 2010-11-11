/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_assign_key_actor.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <string>
#include <utility>

#include "wikiassoc.hpp"

using namespace BOOST_SPIRIT_CLASSIC_NS;
using namespace std;

/*
 * Semantic action functor that stores the title/id pair
 * if the page belongs to the Wikipedia main namespace.
 */
struct AssignTitleActor
{
    unsigned &cur_id, &cur_ns;
    boost::unordered_map<unsigned, unsigned> &wid_to_id;
    boost::unordered_map<string, unsigned> &title_to_wid;
    vector<string> &titles;

    AssignTitleActor(unsigned &id, unsigned &ns,
                     boost::unordered_map<unsigned, unsigned> &widtoid,
                     boost::unordered_map<string, unsigned> &titletowid,
                     vector<string> &ts)
      : cur_id(id), cur_ns(ns), wid_to_id(widtoid),
        title_to_wid(titletowid), titles(ts)
    {
    }

    template <typename Iter>
    void operator()(Iter s, Iter end) const
    {
        if (cur_ns == WIKIPEDIA_MAIN_NS) {
            string title(s,end);
            sql_unescape(title);
            titles.push_back(title);
            wid_to_id[cur_id] = titles.size() - 1;
            title_to_wid[title] = cur_id;
        }
    }
};

/**
 * Grammar for SQL INSERT statement in the MediaWiki `page` table
 *
 * This parser is specific to the MySQL dumps from MediaWiki 1.15,
 * as used by Wikipedia and documented at
 * http://www.mediawiki.org/wiki/Manual:Page_table
 */
struct InsertPage : public grammar<InsertPage>
{
    AssignTitleActor assign_title;
    unsigned cur_id, cur_ns;

    InsertPage(boost::unordered_map<unsigned, unsigned> &widtoid,
               boost::unordered_map<string, unsigned> &titletowid,
               vector<string> &ts)
      : assign_title(cur_id, cur_ns, widtoid, titletowid, ts)
    {
    }

    template <typename Scanner>
    struct definition {
        definition(InsertPage const &self)
        {
            // note: short-circuited |
            strt
                =   *(insert_stmt | comment | other_stmt)
                ;

            comment
                =   comment_p("--") || comment_p("/*", "*/")
               ;

            other_stmt
                =   *(anychar_p - ';')
                 >> ';'
                ;

            insert_stmt
                =   str_p("INSERT") >> str_p("INTO")
                 >> str_p("`page`") >> str_p("VALUES")
                 >> values >> ch_p(';')
                ;

            values
                =   +value % ch_p(',');

            // XXX
            // The const_cast's are ugly; maybe declare the
            // assignment actors in the grammar class?
            value
                =   ch_p('(')
                 >> uint_p[assign_a(const_cast<unsigned &>(self.cur_id))]
                 >> ch_p(',')
                 >> uint_p[assign_a(const_cast<unsigned &>(self.cur_ns))]
                 >> ch_p(',')
                 >> quoted_string
                 >> ch_p(',')
                 >> *(anychar_p - ')')
                 >> ch_p(')')
                ;

            quoted_string
                =   '\''
                 >> quoted_text
                 >> '\''
                ;

            quoted_text
                =   lexeme_d[+qchar][self.assign_title]
                ;

            qchar
                =   (anychar_p - '\\' - '\'')
                |   ('\\' >> anychar_p)
                ;
        }

        rule<Scanner> strt, insert_stmt, other_stmt, values, value,
                  quoted_string, quoted_text, qchar, comment;

        rule<Scanner> const &start() const
        { return strt; }
    };
};

void parse_pagetable(istream &input, 
                     boost::unordered_map<unsigned, unsigned> &wid_to_id,
                     boost::unordered_map<string, unsigned> &title_to_wid,
                     vector<string> &titles)
{
    namespace spirit = boost::spirit;

    input.unsetf(ios::skipws);
    logmsg("parsing page table");

    parse_info<spirit::istream_iterator> info;
    info = parse(spirit::istream_iterator(input),
                 spirit::istream_iterator(),
                 InsertPage(wid_to_id, title_to_wid, titles), space_p);
}
