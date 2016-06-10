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

#include <QLineEdit>
#include <QPainter>
#include <QVBoxLayout>
#include <QLayoutItem>

#include <configuration/iconfiguration.hpp>

#include "config_keys.hpp"
#include "info_widget.hpp"
#include "models/db_data_model.hpp"
#include "ui_utils/photos_item_delegate.hpp"
#include "views/images_tree_view.hpp"


PhotosWidget::PhotosWidget(QWidget* p):
    QWidget(p),
    m_timer(),
    m_thumbnailAcquisitor(),
    m_model(nullptr),
    m_view(nullptr),
    m_delegate(nullptr),
    m_searchExpression(nullptr),
    m_bottomHintLayout(nullptr),
    m_bottomTabBar(nullptr)
{
    auto thumbUpdate = std::bind(&PhotosWidget::thumbnailUpdated, this, std::placeholders::_1, std::placeholders::_2);
    const QImage image(":/gui/clock.svg");
    m_thumbnailAcquisitor.setInProgressThumbnail(image);
    m_thumbnailAcquisitor.setObserver(thumbUpdate);

    // photos view
    m_view = new ImagesTreeView(this);
    m_delegate = new PhotosItemDelegate(m_view);

    m_delegate->set(&m_thumbnailAcquisitor);
    m_view->setItemDelegate(m_delegate);

    // search panel
    QLabel* searchPrompt = new QLabel(tr("Search:"), this);
    m_searchExpression = new QLineEdit(this);

    QHBoxLayout* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(searchPrompt);
    searchLayout->addWidget(m_searchExpression);

    // bottom tools
    const int thumbnailSize = m_view->getThumbnailHeight();

    QLabel* zoomLabel = new QLabel(tr("Thumbnail size:"), this);
    QSlider* zoomSlider = new QSlider(this);
    QLabel* zoomSizeLabel = new QLabel(this);

    zoomSlider->setOrientation(Qt::Horizontal);
    zoomSlider->setMinimum(40);
    zoomSlider->setMaximum(400);
    zoomSlider->setSingleStep(10);
    zoomSlider->setTickInterval(20);
    zoomSlider->setPageStep(30);
    zoomSlider->setValue(thumbnailSize);
    zoomSlider->setTickPosition(QSlider::TicksBelow);

    m_bottomTabBar = new QTabBar(this);
    m_bottomTabBar->setShape(QTabBar::RoundedSouth);

    QHBoxLayout* bottomTools = new QHBoxLayout;
    bottomTools->addWidget(m_bottomTabBar);
    bottomTools->addStretch(3);
    bottomTools->addWidget(zoomLabel);
    bottomTools->addWidget(zoomSlider, 1);
    bottomTools->addWidget(zoomSizeLabel);
    bottomTools->setSpacing(0);

    auto updateZoomSizeLabel = [zoomSizeLabel](int size)
    {
        const QString t = QString("%1 px").arg(size);
        zoomSizeLabel->setText(t);
    };

    updateZoomSizeLabel(thumbnailSize);

    // hint layout
    m_bottomHintLayout = new QVBoxLayout;

    // view + hints layout
    QVBoxLayout* view_hints_layout = new QVBoxLayout;
    view_hints_layout->setContentsMargins(0, 0, 0, 0);
    view_hints_layout->setSpacing(0);
    view_hints_layout->addWidget(m_view);
    view_hints_layout->addLayout(bottomTools);
    view_hints_layout->addLayout(m_bottomHintLayout);

    // main layout
    QVBoxLayout* l = new QVBoxLayout(this);
    l->addLayout(searchLayout);
    l->addLayout(view_hints_layout);

    // setup timer
    m_timer.setInterval(500);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &PhotosWidget::applySearchExpression);

    //
    connect(m_searchExpression, &QLineEdit::textEdited, this, &PhotosWidget::searchExpressionChanged);
    connect(m_view, &ImagesTreeView::contentScrolled, this, &PhotosWidget::viewScrolled);
    connect(this, &PhotosWidget::performUpdate, m_view, &ImagesTreeView::refreshView, Qt::QueuedConnection);
    connect(zoomSlider, &QAbstractSlider::valueChanged, [this, updateZoomSizeLabel](int thumbnailHeight)
    {
        updateZoomSizeLabel(thumbnailHeight);
        m_view->setThumbnailHeight(thumbnailHeight);
        m_thumbnailAcquisitor.dismissPendingTasks();
    });
}


PhotosWidget::~PhotosWidget()
{

}


void PhotosWidget::set(ITaskExecutor* executor)
{
    m_thumbnailAcquisitor.set(executor);
}


void PhotosWidget::set(IPhotosManager* manager)
{
    m_thumbnailAcquisitor.set(manager);
}


void PhotosWidget::set(IConfiguration* configuration)
{
    const QVariant marginEntry = configuration->getEntry(ViewConfigKeys::itemsSpacing);
    assert(marginEntry.isValid());
    const int spacing = marginEntry.toInt();

    m_view->setSpacing(spacing);

    m_delegate->set(configuration);
}


void PhotosWidget::setModel(DBDataModel* m)
{
    m_model = m;
    m_view->setModel(m);
}


QItemSelectionModel* PhotosWidget::viewSelectionModel()
{
    return m_view->selectionModel();
}


void PhotosWidget::setBottomHintWidget(InfoBaloonWidget* hintWidget)
{
    if (m_bottomHintLayout->count() > 0)
    {
        assert(m_bottomHintLayout->count() == 1);
        QLayoutItem* item = m_bottomHintLayout->itemAt(0);
        QWidget* widget = item->widget();

        assert(widget != nullptr);
        delete widget;
    }

    if (hintWidget != nullptr)
        m_bottomHintLayout->addWidget(hintWidget);
}


QTabBar* PhotosWidget::getBottomTabBar() const
{
    return m_bottomTabBar;
}


void PhotosWidget::searchExpressionChanged(const QString &)
{
    m_timer.start();
}


void PhotosWidget::viewScrolled()
{
    m_thumbnailAcquisitor.dismissPendingTasks();
}


void PhotosWidget::applySearchExpression()
{
    const QString search = m_searchExpression->text();

    m_model->applyFilters(search);
}


void PhotosWidget::thumbnailUpdated(const ThumbnailInfo &, const QImage &)
{
    // TODO: do it smarter (find QModelIndex for provided info)
    emit performUpdate();
}
