/*
 * Photo Broom - photos management tool.
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

#include <cassert>

#include "photo_data.hpp"

namespace Photo
{
    Data& Data::apply(const DataDelta& delta)
    {
        id = delta.getId();

        if (delta.has(Photo::Field::Tags))
            tags = delta.get<Photo::Field::Tags>();

        if (delta.has(Photo::Field::Geometry))
            geometry = delta.get<Photo::Field::Geometry>();

        if (delta.has(Photo::Field::Checksum))
            sha256Sum = delta.get<Photo::Field::Checksum>();

        if (delta.has(Photo::Field::Flags))
            flags = delta.get<Photo::Field::Flags>();

        if (delta.has(Photo::Field::GroupInfo))
            groupInfo = delta.get<Photo::Field::GroupInfo>();

        if (delta.has(Photo::Field::Path))
            path = delta.get<Photo::Field::Path>();

        return *this;
    }


    QVariantMap Data::getFlags() const
    {
        QVariantMap result;

        for(const auto& flag: flags)
        {
            const QString id = QString::number(static_cast<int>(flag.first));
            result[id] = flag.second;
        }

        return result;
    }


    void DataDelta::setId(const Photo::Id& id)
    {
        assert(m_id.valid() == false);      // do we expect id to be set more than once?
        m_id = id;
    }


    void DataDelta::clear()
    {
        m_data.clear();
        m_id = Photo::Id();
    }


    bool DataDelta::has(Photo::Field field) const
    {
        auto it = m_data.find(field);

        return it != m_data.end();
    }


    const Id & DataDelta::getId() const
    {
        return m_id;
    }


    bool DataDelta::operator<(const DataDelta& other) const
    {
        return m_id < other.m_id;
    }


    bool DataDelta::operator==(const DataDelta& other) const
    {
        return std::tie(m_id, m_data) == std::tie(other.m_id, other.m_data);
    }


    DataDelta& DataDelta::operator|=(const DataDelta& other)
    {
        assert(m_id.valid() == false || m_id == other.m_id);

        m_id = other.m_id;

        for(const auto& otherData: other.m_data)
        {
            if (otherData.first == Field::Flags && has(Field::Flags))
            {
                auto& flags = std::get<DeltaTypes<Field::Flags>::Storage>(m_data[Photo::Field::Flags]);
                const auto& otherFlags = std::get<DeltaTypes<Field::Flags>::Storage>(otherData.second);

                flags.insert(otherFlags.begin(), otherFlags.end());

            }
            else
                m_data.insert(otherData);
        }

        return *this;
    }


    const DataDelta::Storage& DataDelta::get(Photo::Field field) const
    {
        assert(has(field));
        auto it = m_data.find(field);

        return it->second;
    }
}
