/*
 * Small modification od DBDataModel for main view purposes.
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

#include "main_view_data_model.hpp"

#include <memory>

#include <database/filter.hpp>

MainViewDataModel::MainViewDataModel(QObject* p): DBDataModel(p)
{

}


MainViewDataModel::~MainViewDataModel()
{

}


std::vector<Database::IFilter::Ptr> MainViewDataModel::getModelSpecificFilters() const
{
    auto filter = std::make_shared<Database::FilterFlags>();
    filter->stagingArea = false;

    const std::vector<Database::IFilter::Ptr> result({filter});

    return result;
}