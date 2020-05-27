
#include <core/base_tags.hpp>
#include "common.hpp"


struct PhotosChangeLog: Tests::DatabaseTest
{

};


TEST_F(PhotosChangeLog, tagsManipulation)
{
    for_all_db_plugins([](Database::IBackend* op)
    {
        // add new photo
        Photo::DataDelta pd;
        pd.insert<Photo::Field::Path>("photo1.jpeg");

        std::vector<Photo::DataDelta> photos = { pd };
        ASSERT_TRUE(op->addPhotos(photos));

        // read photo structure
        const Photo::Id id = photos.front().getId();
        Photo::DataDelta data_delta(id);

        // add tags
        Tag::TagsList tags;
        tags[TagTypeInfo(TagTypes::Event)] = TagValue(QString("test event"));

        data_delta.insert<Photo::Field::Tags>(tags);
        op->update(data_delta);

        tags[TagTypeInfo(TagTypes::Place)] = TagValue(QString("test place"));

        data_delta.insert<Photo::Field::Tags>(tags);
        op->update(data_delta);

        // modify tag
        tags[TagTypeInfo(TagTypes::Event)] = TagValue(QString("test event 2"));

        data_delta.insert<Photo::Field::Tags>(tags);
        op->update(data_delta);

        // remove tag
        tags.erase(TagTypeInfo(TagTypes::Place));

        data_delta.insert<Photo::Field::Tags>(tags);
        op->update(data_delta);

        // verify change log
        const QStringList changeLog = op->photoChangeLogOperator().dumpChangeLog();

        ASSERT_EQ(changeLog.size(), 4);
        EXPECT_EQ(changeLog[0], QString("photo id: %1. Tag added. Event: test event").arg(id));
        EXPECT_EQ(changeLog[1], QString("photo id: %1. Tag added. Place: test place").arg(id));
        EXPECT_EQ(changeLog[2], QString("photo id: %1. Tag modified. Event: test event -> test event 2").arg(id));
        EXPECT_EQ(changeLog[3], QString("photo id: %1. Tag removed. Place: test place").arg(id));
    });
}


TEST_F(PhotosChangeLog, groupsManipulation)
{
    for_all_db_plugins([](Database::IBackend* op)
    {
        // add some photos
        Photo::DataDelta pd0,pd1,pd2,pd3,pd4,pd5;

        pd0.insert<Photo::Field::Path>("photo0.jpeg");
        pd1.insert<Photo::Field::Path>("photo1.jpeg");
        pd2.insert<Photo::Field::Path>("photo2.jpeg");
        pd3.insert<Photo::Field::Path>("photo1.jpeg");
        pd4.insert<Photo::Field::Path>("photo4.jpeg");
        pd5.insert<Photo::Field::Path>("photo5.jpeg");

        std::vector<Photo::DataDelta> photos = { pd0, pd1, pd2, pd3, pd4, pd5 };
        ASSERT_TRUE(op->addPhotos(photos));

        const Photo::Id id0 = photos[0].getId();
        const Photo::Id id1 = photos[1].getId();
        const Photo::Id id2 = photos[2].getId();
        const Photo::Id id3 = photos[3].getId();

        // create groups
        const Group::Id gr1 = op->groupOperator().addGroup(id0, Group::Animation);
        const Group::Id gr2 = op->groupOperator().addGroup(id3, Group::Animation);

        // read photo structure
        Photo::DataDelta data_delta1(id1);
        Photo::DataDelta data_delta2(id2);

        // add to group
        const GroupInfo gr_info1 = { gr1, GroupInfo::Member };
        data_delta1.insert<Photo::Field::GroupInfo>(gr_info1);
        op->update(data_delta1);

        const GroupInfo gr_info2 = { gr1, GroupInfo::Member };
        data_delta2.insert<Photo::Field::GroupInfo>(gr_info2);
        op->update(data_delta2);

        // delete group
        op->groupOperator().removeGroup(gr1);
        op->groupOperator().removeGroup(gr2);

        // verify change log
        const QStringList changeLog = op->photoChangeLogOperator().dumpChangeLog();

        ASSERT_EQ(changeLog.size(), 8);
        EXPECT_EQ(changeLog[0], QString("photo id: %1. Group added. %2: 1").arg(id0).arg(gr1));   // photo #1 added to group #1 as representative (1)
        EXPECT_EQ(changeLog[1], QString("photo id: %1. Group added. %2: 1").arg(id3).arg(gr2));   // photo #4 added to group #2 as representative (1)
        EXPECT_EQ(changeLog[2], QString("photo id: %1. Group added. %2: 2").arg(id1).arg(gr1));   // photo #2 added to group #1 as member (2)
        EXPECT_EQ(changeLog[3], QString("photo id: %1. Group added. %2: 2").arg(id2).arg(gr1));   // photo #3 added to group #1 as member (2)
        EXPECT_EQ(changeLog[4], QString("photo id: %1. Group removed. %2: 1").arg(id0).arg(gr1)); // photo #1 removed from group #1 (representative)
        EXPECT_EQ(changeLog[5], QString("photo id: %1. Group removed. %2: 2").arg(id1).arg(gr1)); // photo #2 removed from group #1 (member)
        EXPECT_EQ(changeLog[6], QString("photo id: %1. Group removed. %2: 2").arg(id2).arg(gr1)); // photo #3 removed from group #1 (member)
        EXPECT_EQ(changeLog[7], QString("photo id: %1. Group removed. %2: 1").arg(id3).arg(gr2)); // photo #4 removed from group #2 (representative)
    });
}
