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

#include "staged_photos_widget.hpp"

#include <QVBoxLayout>

#include "staged_photos_data_model.hpp"
#include "ui/photos_view.hpp"


StagedPhotosWidget::StagedPhotosWidget(QWidget* p): QWidget(p),
                                                    m_view(new PhotosView(this)),
                                                    m_dataModel(new StagedPhotosDataModel(this))
{
    QVBoxLayout* l = new QVBoxLayout(this);
    l->addWidget(m_view);

    m_view->setModel(m_dataModel.get());
}


StagedPhotosWidget::~StagedPhotosWidget()
{

}


StagedPhotosDataModel* StagedPhotosWidget::model() const
{
    return m_dataModel.get();
}


void StagedPhotosWidget::set(IConfiguration* configuration)
{
    m_view->set(configuration);
}
