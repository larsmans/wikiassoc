/*
 * Copyright 2010-2011 Lars Buitinck
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
#include <istream>
#include <string>
#include <utility>

#include "wikiassoc.hpp"

#include "article.hpp"
#include "matrix.hpp"

namespace spc = BOOST_SPIRIT_CLASSIC_NS;

/*
 * Semantic action functor that stores the from_id/to_id pair in the
 * supplied containers, if both pages belong to the Wikipedia main
 * namespace and the page being linked to is in titles
 * (is not a 'red link')
 */
struct AssignLinkActor
{
    ArticleSet &articles;
    Matrix &mat;
    std::vector<unsigned> &incoming;
    unsigned &cur_from, &cur_ns;

    AssignLinkActor(unsigned &from, unsigned &ns, ArticleSet &arts, Matrix &m,
                    std::vector<unsigned> &incoming_)
      : articles(arts), cur_from(from), cur_ns(ns), mat(m),
        incoming(incoming_)
    {
    }

    template <typename Iter>
    void operator()(Iter s, Iter end) const
    {
        if (cur_ns != WIKIPEDIA_MAIN_NS)
            return;

        std::string title(s,end);
        sql_unescape(title);

        typedef ArticleSet::index<by_title>::type ArticleByTitle;
        ArticleByTitle::iterator to = articles.get<by_title>().find(title);
        if (to == articles.get<by_title>().end())
            return;

        typedef ArticleSet::index<by_db_id>::type ArticleById;
        ArticleById::iterator from = articles.get<by_db_id>().find(cur_from);
        if (from == articles.get<by_db_id>().end())
            return;

        unsigned from_idx = articles.project<0>(from) - articles.begin(),
                 to_idx   = articles.project<0>(to)   - articles.begin();

        mat(from_idx, to_idx) = 1.;
        incoming[to_idx] += 1;
    }
};

/*
 * Grammar for SQL INSERT statement in the MediaWiki `pagelinks` table
 *
 * This grammar is specific to the MySQL dumps from MediaWiki 1.15,
 * as used by Wikipedia and documented at
 * http://www.mediawiki.org/wiki/Manual:Pagelinks_table
 */
struct InsertLink : public spc::grammar<InsertLink>
{
    AssignLinkActor assign_link;
    unsigned cur_from, cur_ns;

    InsertLink(ArticleSet &articles, Matrix &mat,
               std::vector<unsigned> &incoming)
      : assign_link(cur_from, cur_ns, articles, mat, incoming)
    {
    }

    template <typename Scanner>
    struct definition {
        definition(InsertLink const &self)
        {
            using namespace spc;

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

        spc::rule<Scanner> strt, insert_stmt, other_stmt, values, value,
                          quoted_string, quoted_text, qchar, comment;

        spc::rule<Scanner> const &start() const
        { return strt; }
    };
};

void parse_linktable(std::istream &input, ArticleSet &articles, Matrix &mat,
                     std::vector<unsigned> &incoming)
{
    namespace spirit = boost::spirit;

    input.unsetf(std::ios::skipws);
    logmsg("parsing link table");

    spc::parse_info<spirit::istream_iterator> info;
    info = parse(spirit::istream_iterator(input), spirit::istream_iterator(),
                 InsertLink(articles, mat, incoming), spc::space_p);
}
