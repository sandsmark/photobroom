/*
 * Widget for Staging area.
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

#ifndef STAGEDPHOTOSWIDGET_HPP
#define STAGEDPHOTOSWIDGET_HPP

#include <memory>

#include <QWidget>

class IConfiguration;
class PhotosView;
class StagedPhotosDataModel;

class StagedPhotosWidget : public QWidget
{
    public:
        StagedPhotosWidget(QWidget *);
        StagedPhotosWidget(const StagedPhotosWidget &) = delete;
        ~StagedPhotosWidget();
        StagedPhotosWidget& operator=(const StagedPhotosWidget &) = delete;

        StagedPhotosDataModel* model() const;
        void set(IConfiguration *);

    private:
        PhotosView* m_view;
        std::unique_ptr<StagedPhotosDataModel> m_dataModel;
};

#endif // STAGEDPHOTOSWIDGET_HPP
