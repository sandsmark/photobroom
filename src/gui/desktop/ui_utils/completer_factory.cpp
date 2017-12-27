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

#include "completer_factory.hpp"

#include <QCompleter>

#include <database/database_tools/tag_info_collector.hpp>
#include <utils/tag_value_model.hpp>


namespace
{
    class VariantToStringModelProxy final: public QAbstractListModel
    {
        public:
            VariantToStringModelProxy(QAbstractItemModel* model):
                QAbstractListModel(),
                m_model(model)
            {
            }

            ~VariantToStringModelProxy() = default;

            int rowCount(const QModelIndex &) const override {}
            QVariant data(const QModelIndex &, int) const override {}


        private:
            QAbstractItemModel* m_model;
    };
}


CompleterFactory::CompleterFactory(): m_tagInfoCollector(), m_tagValueModels(), m_loggerFactory(nullptr)
{

}


CompleterFactory::~CompleterFactory()
{

}


void CompleterFactory::set(Database::IDatabase* db)
{
    m_tagInfoCollector.set(db);
}


void CompleterFactory::set(ILoggerFactory* lf)
{
    m_loggerFactory = lf;
}


QCompleter* CompleterFactory::createCompleter(const TagNameInfo& info)
{
    return createCompleter( std::set<TagNameInfo>({info}) );
}


QCompleter* CompleterFactory::createCompleter(const std::set<TagNameInfo>& infos)
{
    QAbstractItemModel* model = getModelFor(infos);

    QCompleter* result = new QCompleter(model);
    return result;
}


QAbstractItemModel* CompleterFactory::getModelFor(const std::set<TagNameInfo>& infos)
{
    auto it = m_tagValueModels.find(infos);

    if (it == m_tagValueModels.end())
    {
        assert(m_loggerFactory != nullptr);

        auto tags_model = std::make_unique<TagValueModel>(infos);
        tags_model->set(m_loggerFactory);
        tags_model->set(&m_tagInfoCollector);

        auto proxy_model = std::make_unique<VariantToStringModelProxy>(tags_model.get());

        ModelPair models = ModelPair( std::move(proxy_model), std::move(tags_model) );
        auto insert_it = m_tagValueModels.insert( std::make_pair(infos, std::move(models)) );

        it = insert_it.first;
    }

    return it->second.first.get();
}
