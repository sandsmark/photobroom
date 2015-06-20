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

#ifndef STAGINGDATAMODEL_H
#define STAGINGDATAMODEL_H

#include "images_model_view/db_data_model.hpp"

class QObject;

class PhotosDataModel: public DBDataModel
{
    public:
        PhotosDataModel(QObject * = nullptr);
        PhotosDataModel(const PhotosDataModel &) = delete;
        ~PhotosDataModel();

        PhotosDataModel& operator=(const PhotosDataModel &) = delete;
};

#endif // STAGINGDATAMODEL_H
