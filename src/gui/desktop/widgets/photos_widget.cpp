/*
 * Widget for Photos
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

#include "photos_widget.hpp"


#include <QPainter>

#include "models/db_data_model.hpp"
#include "info_widget.hpp"


PhotosWidget::PhotosWidget(QWidget* p): ImagesTreeView(p), m_dataModel(nullptr), m_info(nullptr)
{
    m_info = new InfoBaloonWidget(this);
    m_info->hide();
    m_info->setText(tr("There are no photos in your collection.\n\nAdd some by choosing 'Add photos' action from 'Photos' menu."));
    m_info->adjustSize();
}


PhotosWidget::~PhotosWidget()
{

}


void PhotosWidget::paintEvent(QPaintEvent* event)
{
    ImagesTreeView::paintEvent(event);

    // check if model is empty
    QAbstractItemModel* m = model();

    const bool children = m->hasChildren();
    const bool loaded = m->canFetchMore(QModelIndex()) == false;

    const bool empty = children == false || loaded == false;

    if (empty)
    {
        QPixmap infoPixMap(m_info->size());
        infoPixMap.fill(QColor(0, 0, 0, 0));
        m_info->render(&infoPixMap, QPoint(), QRegion(), 0);

        const QRect thisRect = rect();
        QRect infoRect = m_info->rect();
        infoRect.moveCenter(thisRect.center());

        QPainter painter(viewport());
        painter.drawPixmap(infoRect, infoPixMap);
    }
}
