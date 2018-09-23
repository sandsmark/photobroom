/*
 * Factory for QCompleter for particular tag type
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

#ifndef COMPLETERFACTORY_HPP
#define COMPLETERFACTORY_HPP

#include <map>

#include <core/tag.hpp>
#include <database/database_tools/tag_info_collector.hpp>
#include "icompleter_factory.hpp"

class QAbstractItemModel;

namespace Database
{
    struct IDatabase;
}

struct ILoggerFactory;

class CompleterFactory: public ICompleterFactory
{
    public:
        CompleterFactory();
        CompleterFactory(const CompleterFactory &) = delete;
        ~CompleterFactory();
        CompleterFactory& operator=(const CompleterFactory &) = delete;

        void set(Database::IDatabase *);
        void set(ILoggerFactory *);

        QCompleter* createCompleter(const TagNameInfo &) override;
        QCompleter* createCompleter(const std::set<TagNameInfo> &) override;
        QCompleter* createPeopleCompleter() override;

    private:
        typedef std::unique_ptr<QAbstractItemModel> ModelPtr;
        typedef std::pair<ModelPtr, ModelPtr> ModelPair;

        TagInfoCollector m_tagInfoCollector;
        std::map<std::set<TagNameInfo>, ModelPair> m_tagValueModels;
        QAbstractItemModel* m_peopleListModel;
        ILoggerFactory* m_loggerFactory;

        QAbstractItemModel* getModelFor(const std::set<TagNameInfo> &);
        QAbstractItemModel* getModelForPeople();
};

#endif // COMPLETERFACTORY_HPP
