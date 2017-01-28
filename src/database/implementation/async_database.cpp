/*
 * Database thread.
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

#include "async_database.hpp"

#include <thread>
#include <memory>

#include <OpenLibrary/putils/ts_queue.hpp>

#include "ibackend.hpp"
#include "iphoto_info_cache.hpp"
#include "photo_data.hpp"
#include "photo_info.hpp"
#include "photo_info_storekeeper.hpp"
#include "project_info.hpp"


namespace
{

    struct Executor;

    struct IThreadTask
    {
        virtual ~IThreadTask() {}
        virtual void execute(Executor *) = 0;
    };

    struct Executor: Database::ADatabaseSignals
    {
        Executor( std::unique_ptr<Database::IBackend>&& backend, PhotoInfoStorekeeper* storekeeper):
            m_tasks(1024),
            m_backend( std::move(backend) ),
            m_cache(nullptr),
            m_storekeeper(storekeeper)
        {

        }

        Executor(const Executor &) = delete;
        Executor& operator=(const Executor &) = delete;

        virtual ~Executor() {}

        void set(Database::IPhotoInfoCache* cache)
        {
            m_cache = cache;
        }

        void begin()
        {
            for(;;)
            {
                ol::Optional< std::unique_ptr<IThreadTask> > task = m_tasks.pop();

                if (task)
                {
                    IThreadTask* baseTask = task->get();
                    baseTask->execute(this);
                }
                else
                    break;
            }
        }

        void stop()
        {
            m_tasks.stop();
        }

        void closeConnections()
        {
            m_backend->closeConnections();
        }

        IPhotoInfo::Ptr constructPhotoInfo(const Photo::Data& data)
        {
            IPhotoInfo::Ptr photoInfo = std::make_shared<PhotoInfo>(data);

            m_cache->introduce(photoInfo);
            m_storekeeper->photoInfoConstructed(photoInfo);

            return photoInfo;
        }

        IPhotoInfo::Ptr getPhotoFor(const Photo::Id& id)
        {
            IPhotoInfo::Ptr photoPtr = m_cache->find(id);

            if (photoPtr.get() == nullptr)
            {
                const Photo::Data photoData = m_backend->getPhoto(id);

                photoPtr = constructPhotoInfo(photoData);
            }

            return photoPtr;
        }

        std::vector<Photo::Id> insertPhotos(const std::set<QString>& paths)
        {
            std::vector<Photo::Id> result;

            std::deque<Photo::Data> data_set;

            for(const QString& path: paths)
            {
                Photo::Data data;
                data.path = path;
                data.flags[Photo::FlagsE::StagingArea] = 1;

                data_set.push_back(data);
            }

            const bool status = m_backend->addPhotos(data_set);

            if (status)
            {
                std::deque<IPhotoInfo::Ptr> photos;

                for(std::size_t i = 0; i < data_set.size(); i++)
                {
                    const Photo::Data& data = data_set[i];
                    IPhotoInfo::Ptr photoInfo = constructPhotoInfo(data);
                    photos.push_back(photoInfo);

                    result.push_back(data.id);
                }

                emit photosAdded(photos);
            }

            return result;
        }

        Database::IBackend* getBackend() const
        {
            return m_backend.get();
        }

        void addTask(std::unique_ptr<IThreadTask>&& task)
        {
            m_tasks.push(std::move(task));
        }

        private:
            ol::TS_Queue<std::unique_ptr<IThreadTask>> m_tasks;
            std::unique_ptr<Database::IBackend> m_backend;
            Database::IPhotoInfoCache* m_cache;
            PhotoInfoStorekeeper* m_storekeeper;
    };


    struct CreateGroupTask: IThreadTask
    {
        CreateGroupTask(const Photo::Id& representative, const std::function<void(Group::Id)>& callback):
            IThreadTask(),
            m_representative(representative),
            m_callback(callback)
        {

        }

        virtual ~CreateGroupTask() {}

        virtual void execute(Executor* executor) override
        {
            const Group::Id id = executor->getBackend()->addGroup(m_representative);

            if (m_callback)
                m_callback(id);
        }

        Photo::Id m_representative;
        std::function<void(Group::Id)> m_callback;
    };


    struct GetPhotoTask: IThreadTask
    {
        GetPhotoTask(const std::vector<Photo::Id>& ids, const std::function<void(const std::deque<IPhotoInfo::Ptr> &)>& callback):
            IThreadTask(),
            m_ids(ids),
            m_callback(callback)
        {

        }

        virtual ~GetPhotoTask() {}

        virtual void execute(Executor* executor) override
        {
            std::deque<IPhotoInfo::Ptr> photos;

            for (const Photo::Id& id: m_ids)
            {
                IPhotoInfo::Ptr photo = executor->getPhotoFor(id);
                photos.push_back(photo);
            }

            m_callback(photos);
        }

        std::vector<Photo::Id> m_ids;
        std::function<void(const std::deque<IPhotoInfo::Ptr> &)> m_callback;
    };


    struct GetPhotosTask: IThreadTask
    {
        GetPhotosTask (const std::deque<Database::IFilter::Ptr>& filter, const Database::IDatabase::Callback<const IPhotoInfo::List &>& callback):
            IThreadTask(),
            m_filter(filter),
            m_callback(callback)
        {

        }

        virtual ~GetPhotosTask() {}

        virtual void execute(Executor* executor) override
        {
            auto photos = executor->getBackend()->getPhotos(m_filter);
            IPhotoInfo::List photosList;

            for(const Photo::Id& id: photos)
                photosList.push_back(executor->getPhotoFor(id));

            m_callback(photosList);
        }


        std::deque<Database::IFilter::Ptr> m_filter;
        Database::IDatabase::Callback<const IPhotoInfo::List &> m_callback;
    };

    struct InitTask: IThreadTask
    {
        InitTask(const Database::ProjectInfo& prjInfo, const std::function<void(const Database::BackendStatus &)>& callback):
            IThreadTask(),
            m_callback(callback),
            m_prjInfo(prjInfo)
        {

        }

        virtual ~InitTask() {}

        virtual void execute(Executor* executor) override
        {
            const Database::BackendStatus status = executor->getBackend()->init(m_prjInfo);
            m_callback(status);
        }

        std::function<void(const Database::BackendStatus &)> m_callback;
        Database::ProjectInfo m_prjInfo;
    };


    struct InsertPhotosTask: IThreadTask
    {
        InsertPhotosTask(const std::set<QString>& paths, const std::function<void(const std::vector<Photo::Id> &)>& callback):
            IThreadTask(),
            m_paths(paths),
            m_callback(callback)
        {

        }

        virtual ~InsertPhotosTask() {}

        virtual void execute(Executor* executor) override
        {
            const std::vector<Photo::Id> result = executor->insertPhotos(m_paths);

            if (m_callback)
                m_callback(result);
        }

        std::set<QString> m_paths;
        std::function<void(const std::vector<Photo::Id> &)> m_callback;
    };


    struct ListTagValuesTask: IThreadTask
    {
        ListTagValuesTask (const TagNameInfo& info,
                           const std::deque<Database::IFilter::Ptr>& filter,
                           const Database::IDatabase::Callback<const TagNameInfo &, const std::deque<TagValue> &> & callback):
        IThreadTask(),
        m_callback(callback),
        m_info(info),
        m_filter(filter)
        {

        }

        virtual ~ListTagValuesTask() {}

        virtual void execute(Executor* executor) override
        {
            auto result = executor->getBackend()->listTagValues(m_info, m_filter);
            m_callback(m_info, result);
        }

        const Database::IDatabase::Callback<const TagNameInfo &, const std::deque<TagValue> &> m_callback;
        TagNameInfo m_info;
        std::deque<Database::IFilter::Ptr> m_filter;
    };


    struct ListTagsTask: IThreadTask
    {
        ListTagsTask (const Database::IDatabase::Callback<const std::deque<TagNameInfo> &> & callback):
            IThreadTask(),
            m_callback(callback)
        {

        }

        virtual ~ListTagsTask() {}

        virtual void execute(Executor* executor) override
        {
            auto result = executor->getBackend()->listTags();
            m_callback(result);
        }

        Database::IDatabase::Callback<const std::deque<TagNameInfo> &> m_callback;
    };


    struct PhotoCountTask: IThreadTask
    {
        PhotoCountTask (const std::deque<Database::IFilter::Ptr>& filter, const std::function<void(int)>& callback):
            IThreadTask(),
            m_callback(callback),
            m_filter(filter)
        {

        }

        virtual void execute(Executor* executor) override
        {
            auto result = executor->getBackend()->getPhotosCount(m_filter);
            m_callback(result);
        }

        std::function<void(int)> m_callback;
        std::deque<Database::IFilter::Ptr> m_filter;
    };


    struct UpdateTask: IThreadTask
    {
        UpdateTask(const Photo::Data& photoData):
            IThreadTask(),
            m_photoData(photoData)
        {

        }

        virtual ~UpdateTask() {}

        virtual void execute(Executor* executor) override
        {
            const bool status = executor->getBackend()->update(m_photoData);
            assert(status);

            IPhotoInfo::Ptr photoInfo = executor->getPhotoFor(m_photoData.id);
            emit executor->photoModified(photoInfo);
        }

        Photo::Data m_photoData;
    };

}


namespace Database
{

    struct AsyncDatabase::Impl
    {
        Impl( std::unique_ptr<IBackend>&& backend):
            m_cache(nullptr),
            m_storekeeper(),
            m_executor(std::move(backend), &m_storekeeper),
            m_thread(),
            m_working(true)
        {
            m_thread = std::thread(&Executor::begin, &m_executor);
        }

        //store task to be executed by thread
        void addTask( IThreadTask* task)
        {
            assert(m_working);
            m_executor.addTask( std::move(std::unique_ptr<IThreadTask>(task)) );
        }

        //store task to be executed by thread
        void addTask(std::unique_ptr<IThreadTask>&& task)
        {
            assert(m_working);
            m_executor.addTask(std::move(task));
        }

        void stopExecutor()
        {
            if (m_working)
            {
                m_working = false;
                m_executor.stop();

                assert(m_thread.joinable());
                m_thread.join();

                m_executor.closeConnections();
            }
        }

        std::unique_ptr<IPhotoInfoCache> m_cache;
        PhotoInfoStorekeeper m_storekeeper;
        Executor m_executor;
        std::thread m_thread;
        bool m_working;
    };


    AsyncDatabase::AsyncDatabase ( std::unique_ptr<IBackend>&& backend ):
        m_impl(nullptr)
    {
        m_impl = std::make_unique<Impl>( std::move(backend));
        m_impl->m_storekeeper.setDatabase(this);
    }


    AsyncDatabase::~AsyncDatabase()
    {
        //terminate thread
        closeConnections();
    }


    void AsyncDatabase::closeConnections()
    {
        m_impl->stopExecutor();
    }



    void AsyncDatabase::set(std::unique_ptr<IPhotoInfoCache>&& cache)
    {
        m_impl->m_cache = std::move(cache);
        m_impl->m_executor.set(m_impl->m_cache.get());
        m_impl->m_storekeeper.setCache(m_impl->m_cache.get());
    }


    ADatabaseSignals* AsyncDatabase::notifier()
    {
        return &m_impl->m_executor;
    }


    void AsyncDatabase::init(const ProjectInfo& prjInfo, const Callback<const BackendStatus &>& callback)
    {
        InitTask* task = new InitTask(prjInfo, callback);
        m_impl->addTask(task);
    }

    void AsyncDatabase::update(const IPhotoInfo::Ptr& photoInfo)
    {
        UpdateTask* task = new UpdateTask(photoInfo->data());
        m_impl->addTask(task);
    }


    void AsyncDatabase::store(const std::set<QString>& paths, const Callback<const std::vector<Photo::Id> &>& callback)
    {
        InsertPhotosTask* task = new InsertPhotosTask(paths, callback);
        m_impl->addTask(task);
    }


    void AsyncDatabase::createGroup(const Photo::Id& id, const Callback<Group::Id>& callback)
    {
        CreateGroupTask* task = new CreateGroupTask(id, callback);
        m_impl->addTask(task);
    }


    void AsyncDatabase::countPhotos(const std::deque<IFilter::Ptr>& filters, const Callback<int>& callback)
    {
        PhotoCountTask* task = new PhotoCountTask(filters, callback);
        m_impl->addTask(task);
    }


    void AsyncDatabase::getPhotos(const std::vector<Photo::Id>& ids, const Callback<std::deque<IPhotoInfo::Ptr>>& callback)
    {
        GetPhotoTask* task = new GetPhotoTask(ids, callback);
        m_impl->addTask(task);
    }


    void AsyncDatabase::listTagNames( const Callback<const std::deque<TagNameInfo> &> & callback)
    {
        ListTagsTask* task = new ListTagsTask(callback);
        m_impl->addTask(task);
    }

    void AsyncDatabase::listTagValues( const TagNameInfo& info, const Callback<const TagNameInfo &, const std::deque<TagValue> &> & callback)
    {
        ListTagValuesTask* task = new ListTagValuesTask (info, std::deque<IFilter::Ptr>(), callback);
        m_impl->addTask(task);
    }

    void AsyncDatabase::listTagValues( const TagNameInfo& info, const std::deque<IFilter::Ptr>& filters, const Callback<const TagNameInfo &, const std::deque<TagValue> &> & callback)
    {
        ListTagValuesTask* task = new ListTagValuesTask (info, filters, callback);
        m_impl->addTask(task);
    }


    void AsyncDatabase::listPhotos(const std::deque<IFilter::Ptr>& filter, const Callback<const IPhotoInfo::List &>& callback)
    {
        auto task = std::make_unique<GetPhotosTask>(filter, callback);
        m_impl->addTask(std::move(task));
    }

}
