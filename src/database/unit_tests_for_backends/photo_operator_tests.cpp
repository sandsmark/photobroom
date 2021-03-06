
#include "database_tools/json_to_backend.hpp"
#include "unit_tests_utils/sample_db.json.hpp"
#include "unit_tests_utils/sample_db2.json.hpp"

#include "common.hpp"

using testing::Contains;


MATCHER_P(IsPhotoWithPath, _path, "") {
    return arg.path == _path;
}

template<typename T>
struct PhotoOperatorTest: DatabaseTest<T>
{

};

TYPED_TEST_SUITE(PhotoOperatorTest, BackendTypes);


TYPED_TEST(PhotoOperatorTest, gettingAllPhotos)
{
    // fill backend with sample data
    Database::JsonToBackend converter(*this->m_backend);
    converter.append(SampleDB::db1);

    const auto photos = this->m_backend->photoOperator().getPhotos({});

    ASSERT_EQ(photos.size(), 3);

    std::vector<Photo::Data> photo_data;
    photo_data.push_back(this->m_backend->getPhoto(photos[0]));
    photo_data.push_back(this->m_backend->getPhoto(photos[1]));
    photo_data.push_back(this->m_backend->getPhoto(photos[2]));

    EXPECT_THAT(photo_data, Contains(IsPhotoWithPath("/some/path1.jpeg")));
    EXPECT_THAT(photo_data, Contains(IsPhotoWithPath("/some/path2.jpeg")));
    EXPECT_THAT(photo_data, Contains(IsPhotoWithPath("/some/path3.jpeg")));
}


TYPED_TEST(PhotoOperatorTest, sortingByTagActionOnPhotos)
{
    // fill backend with sample data
    Database::JsonToBackend converter(*this->m_backend);
    converter.append(SampleDB::db1);

    Database::Actions::SortByTag sort(TagTypes::Event, Qt::AscendingOrder);
    const auto photos = this->m_backend->photoOperator().onPhotos({}, {sort});

    ASSERT_EQ(photos.size(), 3);

    std::vector<Photo::Data> photo_data;
    photo_data.push_back(this->m_backend->getPhoto(photos[0]));
    photo_data.push_back(this->m_backend->getPhoto(photos[1]));
    photo_data.push_back(this->m_backend->getPhoto(photos[2]));

    EXPECT_THAT(photo_data[0], IsPhotoWithPath("/some/path2.jpeg"));
    EXPECT_THAT(photo_data[1], IsPhotoWithPath("/some/path3.jpeg"));
    EXPECT_THAT(photo_data[2], IsPhotoWithPath("/some/path1.jpeg"));
}


TYPED_TEST(PhotoOperatorTest, sortingByTimestampActionOnPhotos)
{
    // fill backend with sample data
    Database::JsonToBackend converter(*this->m_backend);
    converter.append(SampleDB::db2);

    Database::Actions::SortByTimestamp sort(Qt::DescendingOrder);
    const auto photos = this->m_backend->photoOperator().onPhotos({}, {sort});

    ASSERT_EQ(photos.size(), 21);

    for (int idx = 21, p = 0; p < 21; idx--, p++)
    {
        Photo::Data photo_data;
        photo_data = this->m_backend->getPhoto(photos[p]);

        const QString expected_path = QString("/some/path%1.jpeg").arg(idx);
        EXPECT_THAT(photo_data, IsPhotoWithPath(expected_path));
    }
}

