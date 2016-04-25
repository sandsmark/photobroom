/*
 * Tree View for Images.
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

#include "images_tree_view.hpp"

#include <cassert>
#include <iostream>
#include <functional>

#include <QScrollBar>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

#include <core/time_guardian.hpp>

#include "view_impl/data.hpp"
#include "view_impl/positions_calculator.hpp"
#include "view_impl/positions_reseter.hpp"
#include "view_impl/positions_translator.hpp"
#include "tree_item_delegate.hpp"


using namespace std::literals::chrono_literals;


ImagesTreeView::ImagesTreeView(QWidget* _parent): QAbstractItemView(_parent), m_data(new Data), m_viewStatus(nullptr), m_dataDirty(true)
{
    void (QWidget::*update_fn)() = &QWidget::update;
    auto update_event = std::bind(update_fn, viewport());

    m_viewStatus.connect(this, SIGNAL(refreshView()), update_event, 200ms);
    connect(this, &ImagesTreeView::refreshView, [this]()
    {
        m_dataDirty = true;
    });

    setThumbnailSize(120);
}


ImagesTreeView::~ImagesTreeView()
{

}


void ImagesTreeView::setSpacing(int spacing)
{
    m_data->setSpacing(spacing);
}


void ImagesTreeView::setImageMargin(int margin)
{
    m_data->setImageMargin(margin);
}


void ImagesTreeView::setThumbnailSize(int thumbSize)
{
    verticalScrollBar()->setSingleStep(thumbSize / 2);
    horizontalScrollBar()->setSingleStep(thumbSize / 2);
}


QRect ImagesTreeView::childrenSize(const QModelIndex& idx) const
{
    auto it = m_data->get(idx);

    return it->getOverallRect();
}


QModelIndex ImagesTreeView::indexAt(const QPoint& point) const
{
    QModelIndex result;

    if (model() != nullptr)
    {
        const QPoint offset = getOffset();
        const QPoint treePoint = point + offset;
        Data::ModelIndexInfoSet::iterator infoIt = m_data->get(treePoint);
        result = m_data->get(infoIt);
    }

    return result;
}


bool ImagesTreeView::isIndexHidden(const QModelIndex& index) const
{
    (void) index;

    return false;
}


QRect ImagesTreeView::visualRect(const QModelIndex& index) const
{
    const QRect item_rect = getItemRect(index);
    const QPoint offset = getOffset();
    const QRect result = item_rect.translated(-offset);

    return result;
}


QRegion ImagesTreeView::visualRegionForSelection(const QItemSelection& selection) const
{
    QModelIndexList indexes = selection.indexes();
    QRegion result;

    for (const QModelIndex& idx: indexes)
    {
        const QRect rect = visualRect(idx);

        result += rect;
    }

    return result;
}


int ImagesTreeView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}


int ImagesTreeView::verticalOffset() const
{
    return verticalScrollBar()->value();
}


QModelIndex ImagesTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    const QModelIndex current = selectionModel()->currentIndex();
    QModelIndex result = current;

    switch (cursorAction)
    {
        case MoveLeft:  result = m_data->getLeftOf(current);   break;
        case MoveRight: result = m_data->getRightOf(current);  break;
        case MoveDown:  result = m_data->getBottomOf(current); break;
        case MoveUp:    result = m_data->getTopOf(current);    break;

        case MoveHome:  result = m_data->getFirst(current);    break;
        case MoveEnd:   result = m_data->getLast(current);     break;
    }

    return result;
}


void ImagesTreeView::scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint)
{
    QPoint offset;

    const QRect view = viewport()->rect();
    const int height = view.height();
    const QRect item_visual_rect = visualRect(index);
    const QPoint topLeft = item_visual_rect.topLeft();
    const QPoint bottomRight = item_visual_rect.bottomRight();

    if (topLeft.y() < 0)
    {
        const int dy = topLeft.y();
        offset.setY(dy);
    }
    else if (bottomRight.y() > height)
    {
        const int dy = bottomRight.y() - height;
        offset.setY(dy);
    }

    if (offset.isNull() == false)
    {
        const int y = verticalScrollBar()->value();
        const int ny = y + offset.y();
        verticalScrollBar()->setValue(ny);
    }
}


void ImagesTreeView::setSelection(const QRect& _rect, QItemSelectionModel::SelectionFlags flags)
{
    const QRect treeRect = _rect.translated(getOffset());

    const std::deque<QModelIndex> items = findItemsIn(treeRect);
    QItemSelection selection;

    QAbstractItemModel* m = model();

    for (const QModelIndex& item: items)
    {
        Qt::ItemFlags f = m->flags(item);

        if ( (f & Qt::ItemIsSelectable) == Qt::ItemIsSelectable )
            selection.select(item, item);
    }

    selectionModel()->select(selection, flags);
}


void ImagesTreeView::setModel(QAbstractItemModel* m)
{
    //disconnect current model
    QAbstractItemModel* current_model = QAbstractItemView::model();

    if (current_model)
        current_model->disconnect(this);

    //
    QAbstractItemView::setModel(m);
    m_data->set(m);

    if (m != nullptr)
    {
        //connect to model's signals
        connect(m, &QAbstractItemModel::dataChanged, this, &ImagesTreeView::dataChanged, Qt::UniqueConnection);

        connect(m, SIGNAL(modelReset()), this, SLOT(modelReset()), Qt::UniqueConnection);

        connect(m, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(rowsInserted(QModelIndex,int,int)), Qt::UniqueConnection);
        connect(m, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this, SLOT(rowsMoved(QModelIndex,int,int,QModelIndex,int)), Qt::UniqueConnection);
        connect(m, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowsRemoved(QModelIndex,int,int)), Qt::UniqueConnection);
    }

    modelReset();
}


void ImagesTreeView::paintEvent(QPaintEvent *)
{
    TIME_GUARDIAN("ImagesTreeView::paintEvent", 100, "long paint");

    updateView();

    const PositionsTranslator translator(m_data.get());

    QPainter painter(viewport());
    const QPoint offset = getOffset();
    QRect visible_area = viewport()->rect();

    visible_area.moveTo(offset);
    painter.translate(-offset);

    std::deque<QModelIndex> items = findItemsIn(visible_area);

    for (const QModelIndex& item: items)
    {
        Data::ModelIndexInfoSet::iterator infoIt = m_data->get(item);
        const QRect rect = translator.getAbsoluteRect(infoIt);

        const QSize decorationSize(rect.width()  - m_data->getSpacing() * 2,
                                   rect.height() - m_data->getSpacing() * 2);

        QStyleOptionViewItem styleOption = viewOptions();
        styleOption.rect = rect;
        styleOption.features = m_data->isImage(infoIt)? QStyleOptionViewItem::HasDecoration: QStyleOptionViewItem::HasDisplay;
        styleOption.state |= selectionModel()->isSelected(item)? QStyle::State_Selected: QStyle::State_None;
        styleOption.state |= m_data->isExpanded(infoIt)? QStyle::State_Open: QStyle::State_None;
        styleOption.decorationSize = decorationSize;

        QAbstractItemView::itemDelegate()->paint(&painter, styleOption, item);
    }
}


void ImagesTreeView::mouseReleaseEvent(QMouseEvent* e)
{
    QAbstractItemView::mouseReleaseEvent(e);

    QModelIndex item = indexAt(e->pos());
    Data::ModelIndexInfoSet::iterator infoIt = m_data->find(item);

    if (item.isValid() && infoIt.valid() && m_data->isImage(infoIt) == false)
    {
        ModelIndexInfo& info = *infoIt;
        info.expanded = !info.expanded;

        QAbstractItemModel* view_model = QAbstractItemView::model();
        if (view_model->canFetchMore(item))
            view_model->fetchMore(item);

        //reset some positions
        PositionsReseter reseter(view_model, m_data.get());
        reseter.itemChanged(item);

        emit refreshView();
    }
}


void ImagesTreeView::resizeEvent(QResizeEvent* e)
{
    QAbstractItemView::resizeEvent(e);

    //reset all positions
    PositionsReseter reseter(model(), m_data.get());
    reseter.invalidateAll();

    emit refreshView();
}


/// private:


const QRect ImagesTreeView::getItemRect(const QModelIndex& index) const
{
    const PositionsTranslator translator(m_data.get());
    Data::ModelIndexInfoSet::const_iterator infoIt = m_data->cfind(index);

    assert(infoIt.valid());

    return translator.getAbsoluteRect(infoIt);
}


std::deque<QModelIndex> ImagesTreeView::findItemsIn(const QRect& _rect)
{
    updateView();

    QRect normalized_rect = _rect.normalized();

    const std::deque<QModelIndex> result = m_data->findInRect(normalized_rect);

    return result;
}


void ImagesTreeView::updateData()
{
    QAbstractItemModel* m = QAbstractItemView::model();

    // is there anything to calculate?
    if (m != nullptr && m->rowCount() > 0)
    {
        PositionsCalculator calculator(m_data.get(), viewport()->width());
        calculator.updateItems();
    }
}


void ImagesTreeView::updateGui()
{
    Data::ModelIndexInfoSet::const_iterator infoIt = m_data->cfind(QModelIndex());
    assert(infoIt.valid());

    const ModelIndexInfo& info = *infoIt;
    const QSize areaSize = viewport()->size();
    const QSize treeAreaSize = info.getOverallSize();

    verticalScrollBar()->setPageStep(areaSize.height());
    horizontalScrollBar()->setPageStep(areaSize.width());

    verticalScrollBar()->setRange(0, treeAreaSize.height() - areaSize.height());
    horizontalScrollBar()->setRange(0, treeAreaSize.width() - areaSize.width());
}


QPoint ImagesTreeView::getOffset() const
{
    const QPoint offset(horizontalOffset(), verticalOffset());

    return offset;
}


void ImagesTreeView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& /*roles*/)
{
    const QItemSelection items(topLeft, bottomRight);

    PositionsReseter reseter(model(), m_data.get());
    reseter.itemsChanged(items);

    emit refreshView();
}


void ImagesTreeView::modelReset()
{
    m_data->getModel().modelReset();
}


void ImagesTreeView::rowsInserted(const QModelIndex& _parent, int from, int to)
{
    TIME_GUARDIAN("ImagesTreeView::rowsInserted", 100, "long inserting");

    m_data->getModel().rowsInserted(_parent, from, to);

    PositionsReseter reseter(model(), m_data.get());
    reseter.itemsAdded(_parent, from, to);

    emit refreshView();
}


void ImagesTreeView::rowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow)
{
    TIME_GUARDIAN("ImagesTreeView::rowsMoved", 100, "long moving");

    m_data->getModel().rowsMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow);

    const int items = sourceEnd - sourceStart + 1;

    //reset sizes and positions of existing items
    PositionsReseter reseter(model(), m_data.get());
    reseter.childrenRemoved(sourceParent, sourceStart);

    // when src and dst parents are the same, watch out!
    // http://doc.qt.io/qt-5/qabstractitemmodel.html#beginMoveRows
    if (sourceParent != destinationParent || sourceStart > destinationRow)
        reseter.itemsAdded(destinationParent, destinationRow, destinationRow + items - 1);
    else
        reseter.itemsAdded(destinationParent, destinationRow - items, destinationRow - 1);   // (destinationRow + items - 1) - items

    emit refreshView();
}


void ImagesTreeView::rowsRemoved(const QModelIndex& _parent, int first, int last)
{
    TIME_GUARDIAN("ImagesTreeView::rowsRemoved", 100, "long removing");

    m_data->getModel().rowsRemoved(_parent, first, last);

    //reset sizes and positions of existing items
    PositionsReseter reseter(model(), m_data.get());
    reseter.childrenRemoved(_parent, first);

    emit refreshView();
}


void ImagesTreeView::updateView()
{
    if (m_dataDirty)
    {
        updateData();
        updateGui();

        m_dataDirty = false;
    }
}