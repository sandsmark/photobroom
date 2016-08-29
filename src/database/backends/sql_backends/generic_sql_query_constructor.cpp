/*
 * Basic implementation of ISqlQueryConstructor interface
 * Copyright (C) 2014  Michał Walenciak <MichalWalenciak@gmail.com>
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

#include "generic_sql_query_constructor.hpp"

#include <assert.h>

#include <QStringList>
#include <QVariant>

#include "query_structs.hpp"

namespace Database
{
    QString join(const std::deque<QString>& strs, const QString& spl)
    {
        QString result;

        for(auto it = strs.begin(); it != strs.end();)
        {
            auto current = it++;

            result += *current;

            if (it != strs.end())
                result += spl;
        }

        return result;
    }


    GenericSqlQueryConstructor::GenericSqlQueryConstructor()
    {

    }


    GenericSqlQueryConstructor::~GenericSqlQueryConstructor()
    {

    }


    QString GenericSqlQueryConstructor::prepareInsertQuery(const InsertQueryData& data) const
    {
        QString result;

        const std::deque<QString>& columns = data.getColumns();

        std::deque<QString> valuePlaceholders = columns;
        for(QString& str: valuePlaceholders)
            str.prepend(":");

        result = "INSERT INTO %1(%2) VALUES(%3)";

        result = result.arg(data.getName());
        result = result.arg(join(columns, ", "));
        result = result.arg(join(valuePlaceholders,", "));

        return result;
    }


    QString GenericSqlQueryConstructor::prepareUpdateQuery(const UpdateQueryData& data) const
    {
        QString result;

        const std::deque<QString>& columns = data.getColumns();
        std::deque<QString> valuePlaceholders = columns;
        for(QString& str: valuePlaceholders)
            str.prepend(":");

        const std::pair<QString, QString>& key = data.getCondition();

        result = "UPDATE %1 SET %2 WHERE %3";
        result = result.arg(data.getName());

        QString assigments;
        assert(columns.size() == valuePlaceholders.size());
        const int s = std::min(columns.size(), valuePlaceholders.size());

        for(int i = 0; i < s; i++)
        {
            assigments += columns[i] + "=" + valuePlaceholders[i];

            if (i + 1 < s)
                assigments += ", ";
        }

        const QString condition(key.first + "=:" + key.first);

        result = result.arg(assigments);
        result = result.arg(condition);

        return result;
    }


    QString GenericSqlQueryConstructor::prepareCreationQuery(const QString& name, const QString& columns) const
    {
        return QString("CREATE TABLE %1(%2);").arg(name).arg(columns);
    }


    QString GenericSqlQueryConstructor::prepareFindTableQuery(const QString& name) const
    {
        return QString("SHOW TABLES LIKE '%1';").arg(name);
    }


    QSqlQuery GenericSqlQueryConstructor::insert(const QSqlDatabase& db, const InsertQueryData& data) const
    {
        const QString insertQuery = prepareInsertQuery(data);
        const std::deque<QString>& columns = data.getColumns();
        const std::deque<QVariant>& values = data.getValues();
        const std::size_t count = std::min(columns.size(), values.size());

        QSqlQuery query(db);
        query.prepare(insertQuery);

        for(std::size_t i = 0; i < count; i++)
            query.bindValue(":" + columns[i], values[i]);

        return query;
    }


    QSqlQuery GenericSqlQueryConstructor::update(const QSqlDatabase& db, const UpdateQueryData& data) const
    {
        const QString updateQuery = prepareUpdateQuery(data);
        const std::deque<QString>& columns = data.getColumns();
        const std::deque<QVariant>& values = data.getValues();
        const std::size_t count = std::min(columns.size(), values.size());

        QSqlQuery query(db);
        query.prepare(updateQuery);

        for(std::size_t i = 0; i < count; i++)
            query.bindValue(":" + columns[i], values[i]);

        query.bindValue(":" + data.getCondition().first, data.getCondition().second);

        return query;
    }

}