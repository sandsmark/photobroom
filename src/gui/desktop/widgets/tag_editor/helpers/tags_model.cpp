/*
 * Tags model
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

/// TODO: remove
#if defined _MSC_VER
    #if _MSC_VER >= 1800
        #define Q_COMPILER_INITIALIZER_LISTS
    #else
        #error unsupported compiler
    #endif
#endif


#include "tags_model.hpp"

#include <QItemSelectionModel>

#include <core/cross_thread_call.hpp>
#include <database/idatabase.hpp>
#include "models/db_data_model.hpp"
#include "tags_operator.hpp"


using namespace std::placeholders;

TagsModel::TagsModel(QObject* p):
    QStandardItemModel(p),
    m_selectionExtractor(),
    m_selectionModel(nullptr),
    m_dbDataModel(nullptr),
    m_tagsOperator(nullptr),
    m_database(nullptr)
{
}


TagsModel::~TagsModel()
{

}


void TagsModel::set(Database::IDatabase* database)
{
    m_database = database;
}


void TagsModel::set(QItemSelectionModel* selectionModel)
{
    m_selectionExtractor.set(selectionModel);

    if (m_selectionModel != nullptr)
        m_selectionModel->disconnect(this);

    m_selectionModel = selectionModel;
    connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(refreshModel(QItemSelection, const QItemSelection &)));
    connect(this, &TagsModel::dataChanged, this, &TagsModel::syncData);
    connect(this, &TagsModel::emptyValueError, this, static_cast<void (TagsModel::*)()>(&TagsModel::refreshModel), Qt::QueuedConnection);   // refresh model on problems

    refreshModel();
}


void TagsModel::set(DBDataModel* dbDataModel)
{
    m_selectionExtractor.set(dbDataModel);
    m_dbDataModel = dbDataModel;
}


void TagsModel::set(ITagsOperator* tagsOperator)
{
    m_tagsOperator = tagsOperator;
}


Tag::TagsList TagsModel::getTags() const
{
    return m_tagsOperator->getTags();
}


void TagsModel::addTag(const TagNameInfo& info, const TagValue& value)
{
    m_tagsOperator->setTag(info, value);

    refreshModel();
}


void TagsModel::refreshModel()
{
    if (m_dbDataModel != nullptr && m_selectionModel != nullptr)
    {
        clearModel();

        std::vector<Photo::Data> photos = m_selectionExtractor.getSelection();

        std::vector<Photo::Id> ids;
        for(const Photo::Data& photo: photos)
            ids.push_back(photo.id);

        Database::IDatabase::Callback<const IPhotoInfo::List &> target_fun =
            std::bind(&TagsModel::loadPhotos, this, _1);

        auto callback = make_cross_thread_function(this, target_fun);

        m_database->getPhotos(ids, callback);
    }
}


void TagsModel::clearModel()
{
    clear();
    setHorizontalHeaderLabels( {tr("Name"), tr("Value")} );
}


void TagsModel::loadPhotos(const std::vector<IPhotoInfo::Ptr>& photos)
{
    m_tagsOperator->operateOn(photos);

    Tag::TagsList tags = getTags();

    for (const auto& tag: tags)
    {
        Tag::Info info(tag);
        QStandardItem* name = new QStandardItem(info.displayName());
        QStandardItem* value = new QStandardItem;

        const QVariant dispRole = info.value().get();
        const QVariant tagInfoRole = QVariant::fromValue(info.getTypeInfo());

        value->setData(dispRole, Qt::DisplayRole);
        value->setData(tagInfoRole, TagInfoRole);

        const QList<QStandardItem *> items( { name, value });
        appendRow(items);
    }

    emit modelChanged(photos.empty() == false);
}


void TagsModel::refreshModel(const QItemSelection &, const QItemSelection &)
{
    refreshModel();
}


void TagsModel::syncData(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    const QItemSelection items(topLeft, bottomRight);
    const QModelIndexList itemsList(items.indexes());

    bool update_failed = false;

    for (const QModelIndex& itemIndex: itemsList)
    {
        assert(itemIndex.column() == 1);

        if (itemIndex.column() == 1)
        {
            const QModelIndex tagNameIndex = itemIndex.sibling(itemIndex.row(), 0);
            const QString tagName = tagNameIndex.data().toString();
            const QVariant valueRaw = itemIndex.data();
            const TagValue value = TagValue::fromQVariant(valueRaw);

            if (value.rawValue().isEmpty())
                update_failed = true;
            else
                m_tagsOperator->updateTag(tagName, value);
        }
    }

    if (update_failed)
        emit emptyValueError();
}
