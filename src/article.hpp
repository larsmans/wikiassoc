/*
 * Copyright 2010 Lars Buitinck
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ARTICLE_HPP
#define ARTICLE_HPP

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <string>

/**
 * Wiki article
 */
struct Article
{
    std::string title;
    unsigned db_id;     // id field in MediaWiki database dump

    Article(std::string const &t, unsigned id) : title(t), db_id(id) { }
};

// Access tags for ArticleSet
struct by_title {};
struct by_db_id {};

/**
 * Collection of wiki articles, indexed numerically, by title and by
 * MediaWiki database id
 */
class ArticleSet : public boost::multi_index_container<
    Article,
    boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,
        boost::multi_index::hashed_unique<
            boost::multi_index::tag<by_db_id>,
            boost::multi_index::member<Article, unsigned, &Article::db_id>
        >,
        boost::multi_index::hashed_unique<
            boost::multi_index::tag<by_title>,
            boost::multi_index::member<Article, std::string, &Article::title>
        >
    >
>
{
};

#endif  // ARTICLE_HPP
