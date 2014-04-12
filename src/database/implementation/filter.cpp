/*
 * <one line to give the program's name and a brief idea of what it does.>
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

#include "filter.hpp"

namespace Database
{

    FilterDescription::FilterDescription(): tagName(), tagValue()
    {

    }


    bool FilterDescription::empty() const
    {
        return tagName.isEmpty();
    }


    Filter::Filter(): m_filters()
    {

    }


    Filter::~Filter()
    {

    }


    void Filter::addFilter(const FilterDescription& filter)
    {
        if (! filter.empty())
            m_filters.push_back(filter);
    }


    const std::vector<FilterDescription>& Filter::getFilters() const
    {
        return m_filters;
    }

}