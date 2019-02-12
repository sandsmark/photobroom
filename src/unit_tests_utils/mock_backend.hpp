
#ifndef MOCK_BACKEND_HPP
#define MOCK_BACKEND_HPP


#include <gmock/gmock.h>

#include <database/filter.hpp>
#include <database/ibackend.hpp>
#include <database/project_info.hpp>


struct MockBackend : public Database::IBackend
{
  MOCK_METHOD1(addPhotos,
      bool(std::vector<Photo::DataDelta> &));
  MOCK_METHOD2(addGroup,
      Group::Id(const Photo::Id &, GroupInfo::Type));
  MOCK_METHOD1(update,
      bool(const Photo::DataDelta &));
  MOCK_METHOD0(listTags,
      std::vector<TagNameInfo>());
  MOCK_METHOD2(listTagValues,
      std::vector<TagValue>(const TagNameInfo &, const std::vector<Database::IFilter::Ptr> &));
  MOCK_METHOD0(getAllPhotos,
      std::vector<Photo::Id>());
  MOCK_METHOD1(getPhotos,
      std::vector<Photo::Id>(const std::vector<Database::IFilter::Ptr> &));
  MOCK_METHOD1(dropPhotos,
      std::vector<Photo::Id>(const std::vector<Database::IFilter::Ptr> &));
  MOCK_METHOD1(getPhoto,
      Photo::Data(const Photo::Id &));
  MOCK_METHOD1(getPhotosCount,
      int(const std::vector<Database::IFilter::Ptr> &));
  MOCK_METHOD0(listPeople,
      std::vector<PersonName>());
  MOCK_METHOD1(listPeople,
      std::vector<PersonInfo>(const Photo::Id &));
  MOCK_METHOD1(person,
      PersonName(const Person::Id &));
  MOCK_METHOD1(store,
      Person::Id(const PersonName &));
  MOCK_METHOD1(store,
      PersonInfo::Id(const PersonInfo &));
  MOCK_METHOD3(set,
      void(const Photo::Id &, const QString &, int value));
  MOCK_METHOD2(get,
      std::optional<int>(const Photo::Id &, const QString &));
  MOCK_METHOD2(perform,
      void(const std::vector<Database::IFilter::Ptr> &, const std::vector<Database::IAction::Ptr> &));
  MOCK_METHOD0(markStagedAsReviewed,
      std::vector<Photo::Id>());
  MOCK_METHOD1(init,
      Database::BackendStatus(const Database::ProjectInfo &));
  MOCK_METHOD0(closeConnections,
      void());
};


#endif