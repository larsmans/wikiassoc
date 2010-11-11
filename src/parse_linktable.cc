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
#include <iostream>
#include <string>
#include <utility>

#include "wikiassoc.hpp"
#include "matrix.hpp"

using namespace BOOST_SPIRIT_CLASSIC_NS;
using namespace std;

/*
 * Semantic action functor that stores the from_id/to_id pair in the
 * supplied containers, if both pages belong to the Wikipedia main
 * namespace and the page being linked to is in titles
 * (is not a 'red link')
 */
struct AssignLinkActor
{
    Matrix &mat;
    boost::unordered_map<unsigned, unsigned> const &wid_to_id;
    boost::unordered_map<string, unsigned> const &title_to_wid;
    vector<unsigned> &incoming;
    unsigned &cur_from, &cur_ns;

    AssignLinkActor(unsigned &from, unsigned &ns, Matrix &m,
                    boost::unordered_map<unsigned, unsigned> const &widtoid,
                    boost::unordered_map<string, unsigned> const &titletoid,
                    vector<unsigned> &incoming_)
      : cur_from(from), cur_ns(ns), mat(m),
        wid_to_id(widtoid), title_to_wid(titletoid), incoming(incoming_)
    {
    }

    template <typename Iter>
    void operator()(Iter s, Iter end) const
    {
        if (cur_ns == WIKIPEDIA_MAIN_NS) {
            string title(s,end);
            sql_unescape(title);
            boost::unordered_map<string, unsigned>::const_iterator to(
                    title_to_wid.find(title)
            );

            if (to != title_to_wid.cend())
                try {
                    mat(wid_to_id.at(cur_from),
                        wid_to_id.at(to->second)) = 1.;
                    incoming[wid_to_id.at(to->second)] += 1;
                } catch (std::out_of_range const &e) {
                }
        }
    }
};

/*
 * Grammar for SQL INSERT statement in the MediaWiki `pagelinks` table
 *
 * This grammar is specific to the MySQL dumps from MediaWiki 1.15,
 * as used by Wikipedia and documented at
 * http://www.mediawiki.org/wiki/Manual:Pagelinks_table
 */
struct InsertLink : public grammar<InsertLink>
{
    AssignLinkActor assign_link;
    unsigned cur_from, cur_ns;

    InsertLink(Matrix &mat,
               boost::unordered_map<unsigned, unsigned> const &widtoid,
               boost::unordered_map<string, unsigned> const &titletoid,
               vector<unsigned> &incoming)
      : assign_link(cur_from, cur_ns, mat, widtoid, titletoid, incoming)
    {
    }

    template <typename Scanner>
    struct definition {
        definition(InsertLink const &self)
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
                 >> str_p("`pagelinks`") >> str_p("VALUES")
                 >> values >> ch_p(';')
                ;

            values
                =   +value % ch_p(',');

            // XXX
            // The const_cast's are ugly; maybe declare the
            // assignment actors in the grammar class?
            value
                =   ch_p('(')
                 >> uint_p[assign_a(const_cast<unsigned &>(self.cur_from))]
                 >> ch_p(',')
                 >> uint_p[assign_a(const_cast<unsigned &>(self.cur_ns))]
                 >> ch_p(',')
                 >> quoted_string
                 >> ch_p(')')
                ;

            quoted_string
                =   '\''
                 >> quoted_text
                 >> '\''
                ;

            quoted_text
                =   lexeme_d[+qchar][self.assign_link]
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

void parse_linktable(istream &input, Matrix &mat,
                     boost::unordered_map<unsigned, unsigned> const &wid_to_id,
                     boost::unordered_map<string, unsigned> const &title_to_wid,
                     vector<unsigned> &incoming)
{
    namespace spirit = boost::spirit;

    input.unsetf(ios::skipws);
    logmsg("parsing link table");

    parse_info<spirit::istream_iterator> info;
    info = parse(spirit::istream_iterator(input),
                 spirit::istream_iterator(),
                 InsertLink(mat, wid_to_id, title_to_wid, incoming), space_p);
}
