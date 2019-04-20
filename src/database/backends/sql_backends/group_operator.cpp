/*
 * Class for performing operations on groups
 * Copyright (C) 2019  Michał Walenciak <Kicer86@gmail.com>
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
 */

#include "group_operator.hpp"

#include <QSqlDatabase>
#include <QSqlQuery>

#include "database/ibackend.hpp"
#include "isql_query_constructor.hpp"
#include "isql_query_executor.hpp"
#include "query_structs.hpp"
#include "tables.hpp"

namespace Database
{

    GroupOperator::GroupOperator(const QString& name,
                                 const IGenericSqlQueryGenerator* generator,
                                 ISqlQueryExecutor* executor,
                                 IBackend* backend):
        m_connectionName(name),
        m_queryGenerator(generator),
        m_executor(executor),
        m_backend(backend)
    {
    }


    Group::Id GroupOperator::addGroup(const Photo::Id& id, Group::Type type)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);

        Group::Id grp_id;

        InsertQueryData insertData(TAB_GROUPS);

        insertData.setColumns("id", "representative_id", "type");
        insertData.setValues(InsertQueryData::Value::Null, id, static_cast<int>(type));

        QSqlQuery query = m_queryGenerator->insert(db, insertData);

        bool status = m_executor->exec(query);

        //update id
        if (status)                                    //Get Id from database after insert
        {
            QVariant group_id  = query.lastInsertId(); //TODO: WARNING: may not work (http://qt-project.org/doc/qt-5.1/qtsql/qsqlquery.html#lastInsertId)
            status = group_id.isValid();

            if (status)
                grp_id = Group::Id(group_id.toInt());

            emit m_backend->photoModified(id);        // photo is now a representative
        }

        return grp_id;
    }


    Photo::Id Database::GroupOperator::removeGroup(const Group::Id& gid)
    {
        Photo::Id representativePhoto;
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);

        try
        {
            const QString query_str =
                QString("SELECT representative_id FROM %1 WHERE id=%2").arg(TAB_GROUPS).arg(gid);

            QSqlQuery query(db);
            DB_ERR_ON_FALSE(m_executor->exec(query_str, &query));
            DB_ERR_ON_FALSE(query.next());

            const Photo::Id ph_id( query.value(0).toInt() );

            // add representative to modified_photos
            // as it won't be part of the group anymore
            std::set<Photo::Id> modified_photos;
            modified_photos.insert(ph_id);

            DB_ERR_ON_FALSE(db.transaction());

            const QString members_list =
                QString("SELECT photo_id FROM %1 WHERE group_id=%2").arg(TAB_GROUPS_MEMBERS).arg(gid);

            DB_ERR_ON_FALSE(m_executor->exec(members_list, &query));

            while(query.next())
            {
                // add members to modified photos
                // as they won't be part of the group anymore

                const Photo::Id mem_id(query.value(0).toInt());
                modified_photos.insert(mem_id);
            }

            const QString members_delete =
                QString("DELETE FROM %1 WHERE group_id=%2").arg(TAB_GROUPS_MEMBERS).arg(gid);

            const QString group_delete =
                QString("DELETE FROM %1 WHERE id=%2").arg(TAB_GROUPS).arg(gid);

            DB_ERR_ON_FALSE(m_executor->exec(members_delete, &query));
            DB_ERR_ON_FALSE(m_executor->exec(group_delete, &query));

            DB_ERR_ON_FALSE(db.commit());

            for(const Photo::Id& id: modified_photos)
                emit m_backend->photoModified(id);

            representativePhoto = ph_id;
        }
        catch(const db_error& ex)
        {
            db.rollback();
        }

        return representativePhoto;
    }


    Group::Type Database::GroupOperator::type(const Group::Id& id) const
    {
        Group::Type type = Group::Invalid;

        QSqlDatabase db = QSqlDatabase::database(m_connectionName);

        const QString query_str =
            QString("SELECT type FROM %1 WHERE id=%2").arg(TAB_GROUPS).arg(id);

        QSqlQuery query(db);
        bool status = m_executor->exec(query_str, &query);

        if (status && query.next())
        {
            const QVariant typeVariant = query.value(0);
            type = static_cast<Group::Type>(typeVariant.toInt());
        }

        return type;
    }
}
