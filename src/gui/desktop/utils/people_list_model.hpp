/*
 * Model keeping list of all people names
 * Copyright (C) 2018  Michał Walenciak <Kicer86@gmail.com>
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

#ifndef PEOPLELISTMODEL_HPP
#define PEOPLELISTMODEL_HPP

#include <QAbstractListModel>

namespace Database
{
    struct IDatabase;
}

class PeopleListModel: public QAbstractListModel
{
    public:
        PeopleListModel();
        ~PeopleListModel();

        void setDB(Database::IDatabase *);

        QVariant data(const QModelIndex & index, int role) const override;
        int rowCount(const QModelIndex & parent) const override;

    private:
        QStringList m_names;

        void fill(const QStringList &);
        void clear();
};

#endif // PEOPLELISTMODEL_HPP