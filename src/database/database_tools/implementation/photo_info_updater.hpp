
#ifndef GUI_PHOTO_INFO_UPDATER_HPP
#define GUI_PHOTO_INFO_UPDATER_HPP

#include <mutex>
#include <condition_variable>

#include <core/tag_feeder_factory.hpp>
#include <core/task_executor.hpp>
#include <database/iphoto_info.hpp>

struct IPhotosManager;
struct IConfiguration;

struct UpdaterTask;


//TODO: construct photo manualy. Add fillers manualy on demand
class PhotoInfoUpdater final
{
    public:
        PhotoInfoUpdater();
        ~PhotoInfoUpdater();

        PhotoInfoUpdater(const PhotoInfoUpdater &) = delete;
        PhotoInfoUpdater& operator=(const PhotoInfoUpdater &) = delete;

        void updateSha256(const IPhotoInfo::Ptr &);
        void updateTags(const IPhotoInfo::Ptr &);

        void set(ITaskExecutor *);
        void set(IConfiguration *);
        void set(IPhotosManager *);

        int tasksInProgress();
        void dropPendingTasks();
        void waitForActiveTasks();

    private:
        friend struct UpdaterTask;

        TagFeederFactory m_tagFeederFactory;
        ITaskExecutor::TaskQueue m_taskQueue;
        std::set<UpdaterTask *> m_tasks;
        std::mutex m_tasksMutex;
        std::condition_variable m_finishedTask;
        IConfiguration* m_configuration;
        IPhotosManager* m_photosManager;

        void taskAdded(UpdaterTask *);
        void taskFinished(UpdaterTask *);
};

#endif
