/*
 * Database observer, tags analyzer and collector
 * Copyright (C) 2016  Michał Walenciak <MichalWalenciak@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TAGINFOCOLLECTOR_HPP
#define TAGINFOCOLLECTOR_HPP

#include <core/tag.hpp>

namespace Database
{
    struct IDatabase;
}

class TagInfoCollector
{
    public:
        TagInfoCollector();
        TagInfoCollector(const TagInfoCollector &) = delete;
        ~TagInfoCollector();

        TagInfoCollector& operator=(const TagInfoCollector &) = delete;

        void set(Database::IDatabase *);
        const std::set<TagValue>& get(const TagNameInfo &) const;

    private:
        mutable std::map<TagNameInfo, std::set<TagValue>> m_tags;
        Database::IDatabase* m_database;

        void gotTagNames(const std::deque<TagNameInfo> &);

    signals:
        void valuesChanged(const TagNameInfo &);
};

#endif // TAGINFOCOLLECTOR_H
