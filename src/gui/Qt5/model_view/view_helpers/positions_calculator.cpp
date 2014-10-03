/*
 * QModelIndex position calculator
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

#include "positions_calculator.hpp"

#include <cassert>

#include <QModelIndex>

#include <configuration/constants.hpp>
#include <configuration/configuration.hpp>

#include "data.hpp"

PositionsCalculator::PositionsCalculator(QAbstractItemModel* model, Data* data, int width): m_model(model), m_data(data), m_width(width), m_itemsPerRow(itemsPerRow())
{

}


PositionsCalculator::~PositionsCalculator()
{

}


void PositionsCalculator::updateItems() const
{
    m_data->for_each_recursively(m_model, [&](const QModelIndex& idx, const std::deque<QModelIndex>& children)
    {
        ModelIndexInfo info = m_data->get(idx);

        // calculations only for dirty ones
        if (info.getRect().isNull())
        {
            QRect rect = calcItemRect(idx);
            info.setRect(rect);
            m_data->update(info);                                        // size muse be stored at this point, as children calculations may require it
        }

        if (info.getOverallRect().isNull())
        {
            QRect rect = info.getRect();
            for(const QModelIndex& child: children)
            {
                ModelIndexInfo c_info = m_data->get(child);
                QRect c_rect = c_info.getRect();
                assert(c_rect.isValid());

                rect = rect.united(c_rect);
            }

            info.setOverallRect(rect);
            m_data->update(info);
        }
    });
}


QRect PositionsCalculator::calcItemRect(const QModelIndex& index) const
{
    QRect result;

    if (index != QModelIndex())
    {
        QModelIndex item_parent = m_model->parent(index);
        const QSize item_size = getItemSize(index);

        if (index.row() == 0)  //first
        {
            const QPoint point = positionOfFirstChild(item_parent);

            result = QRect(point, item_size);
        }
        else
        {
            const QModelIndex sibling = m_model->index(index.row() - 1, 0, item_parent);
            const QPoint point = positionOfNext(sibling);

            result = QRect(point, item_size);
        }
    }

    return result;
}


QPoint PositionsCalculator::positionOfNext(const QModelIndex& index) const
{
    const bool image = m_data->isImage(m_model, index);
    const QPoint result = image? positionOfNextImage(index):
                                 positionOfNextNode(index);

    return result;
}


QPoint PositionsCalculator::positionOfNextImage(const QModelIndex& index) const
{
    assert(index.isValid());

    const QPoint items_matrix_pos = matrixPositionOf(index);
    const ModelIndexInfo& info = m_data->get(index);
    const QRect& items_pos = info.getRect();

    assert(items_matrix_pos.x() < m_itemsPerRow);

    QPoint result;
    if (items_matrix_pos.x() + 1 < m_itemsPerRow)             //not last in its row?
        result = QPoint(items_pos.x() + getitemWidth(index), items_pos.y());
    else                                                      //last in a row
    {
        QModelIndex item_parent = m_model->parent(index);
        QModelIndex from = itemAtMatrixPosition(QPoint(0, items_matrix_pos.y()), item_parent);
        QModelIndex to = itemAtMatrixPosition(QPoint(m_itemsPerRow - 1, items_matrix_pos.y()), item_parent);

        const int item_height = getItemHeigth(from, to);

        result = QPoint(0, items_pos.y() + item_height);
    }

    return result;
}


QPoint PositionsCalculator::positionOfNextNode(const QModelIndex& index) const
{
    assert(index.isValid());

    ModelIndexInfo info = m_data->get(index);
    const QRect items_pos = info.getOverallRect();
    assert(items_pos.isValid());
    const QPoint result = QPoint(0, items_pos.bottom());

    return result;
}


QPoint PositionsCalculator::positionOfFirstChild(const QModelIndex& index) const
{
    QPoint result(0, 0);

    if (index != QModelIndex())           // regular item
    {
        const QRect r = calcItemRect(index);
        result = QPoint(0, r.y() + r.height());
    }

    return result;
}


QPoint PositionsCalculator::matrixPositionOf(const QModelIndex& index) const
{
    assert(index.column() == 0);    // ImagesTreeView supports only typical hierarchical models. So column of item will be always equal to 0

    const int linear_pos = index.row();
    const int row = linear_pos / m_itemsPerRow;
    const int col = linear_pos % m_itemsPerRow;

    return QPoint(col, row);
}


QModelIndex PositionsCalculator::itemAtMatrixPosition(const QPoint& point, QModelIndex& _parent) const
{
    const int liner_pos = m_itemsPerRow * point.y() + point.x();

    QModelIndex item = m_model->index(liner_pos, 0, _parent);

    return item;
}


int PositionsCalculator::itemsPerRow() const
{
    const int indexWidth = m_data->m_configuration->findEntry(Configuration::BasicKeys::thumbnailWidth, "120").toInt() + m_data->indexMargin * 2;
    const int indicesPerRowInitial = m_width / indexWidth;
    const int indicesPerRow = indicesPerRowInitial > 1? indicesPerRowInitial : 2;    // at least 2 items per row

    return indicesPerRow;
}


int PositionsCalculator::getitemWidth(const QModelIndex& index) const
{
    int w = 0;
    if (m_data->isImage(m_model, index))   //image
    {
        QPixmap pixmap = m_data->getImage(m_model, index);
        w = pixmap.width() + m_data->indexMargin;
    }
    else                  //node's title
        w = m_width;

    return w;
}


int PositionsCalculator::getItemHeigth(const QModelIndex& index) const
{
    int item_height = 0;
    if (m_data->isImage(m_model, index))   //image
    {
        QPixmap pixmap = m_data->getImage(m_model, index);
        item_height = pixmap.height() + m_data->indexMargin;
    }
    else                  //node's title
        item_height = 40;      //TODO: temporary

        return item_height;
}


int PositionsCalculator::getItemHeigth(const QModelIndex& from, const QModelIndex& to) const
{
    int result = 0;
    QModelIndex item_parent = m_model->parent(from);
    assert(item_parent == m_model->parent(to));

    for (int i = from.row(); i < to.row(); i++)
    {
        QModelIndex index = m_model->index(i, 0, item_parent);

        const int h = getItemHeigth(index);
        if (result < h)
            result = h;
    }

    return result;
}


void PositionsCalculator::calcItemsOverallRect() const
{
    m_data->for_each_recursively(m_model, [&](const QModelIndex& idx, const std::deque<QModelIndex>& children)
    {
        ModelIndexInfo info = m_data->get(idx);
        QRect rect = info.getRect();

        for(const QModelIndex& child: children)
        {
            ModelIndexInfo c_info = m_data->get(child);
            QRect c_rect = c_info.getOverallRect();
            assert(c_rect.isValid());                            // not valid?  Should never happen

            rect = rect.united(c_rect);
        }

        info.setOverallRect(rect);
        m_data->update(info);
    });
}


QSize PositionsCalculator::getItemSize(const QModelIndex& index) const
{
    const QSize item_size(getitemWidth(index), getItemHeigth(index));

    return item_size;
}
