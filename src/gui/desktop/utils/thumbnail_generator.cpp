/*
 * Thumbnail generator.
 * Copyright (C) 2016  Michał Walenciak <MichalWalenciak@gmail.com>
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

#include "thumbnail_generator.hpp"

#include <core/ilogger.hpp>
#include <core/iphotos_manager.hpp>
#include <core/stopwatch.hpp>
#include <core/task_executor.hpp>
#include <core/iexif_reader.hpp>


struct ThumbnailGenerator::GenerationTask: TaskExecutor::ITask
{
    GenerationTask(const ThumbnailInfo& info,
                            const ThumbnailGenerator::Callback& callback,
                            const ThumbnailGenerator* generator):
        m_info(info),
        m_callback(callback),
        m_generator(generator)
    {

    }

    virtual ~GenerationTask() {}

    GenerationTask(const GenerationTask &) = delete;
    GenerationTask& operator=(const GenerationTask &) = delete;

    virtual std::string name() const override
    {
        return "Photo thumbnail generation";
    }

    virtual void perform() override
    {
        // TODO: use QTransform here to perform one transformation instead of many

        IExifReader* reader = m_generator->m_exifReaderFactory->get();
        const bool needs_to_be_rotated = shouldSwap(reader);

        Stopwatch stopwatch;

        stopwatch.start();
        QByteArray raw = m_generator->m_photosManager->getPhoto(m_info.path);

        QImage image;
        image.loadFromData(raw);
        const int photo_read = stopwatch.read(true);

        if (needs_to_be_rotated)
        {
            if (image.width() != m_info.height)         // because photo will be rotated by 90⁰, use width as it was height
                image = image.scaledToWidth(m_info.height, Qt::SmoothTransformation);
        }
        else if (image.height() != m_info.height)
            image = image.scaledToHeight(m_info.height, Qt::SmoothTransformation);

        const int photo_scaling = stopwatch.stop();

        const std::string read_time_message = std::string("photo read time: ") + std::to_string(photo_read) + "ms";
        m_generator->m_logger->debug(read_time_message);

        const std::string scaling_time_message = std::string("photo scaling time: ") + std::to_string(photo_scaling) + "ms";
        m_generator->m_logger->debug(scaling_time_message);

        image = rotateThumbnail(reader, image );

        m_callback(m_info, image );
    }

    bool shouldSwap(IExifReader* reader)
    {
        const boost::any orientation_raw = reader->get(m_info.path, IExifReader::TagType::Orientation);
        const int orientation = boost::any_cast<int>(orientation_raw);

        return orientation > 4;
    }

    QImage rotateThumbnail(IExifReader* reader, const QImage& thumbnail) const
    {
        const boost::any orientation_raw = reader->get(m_info.path, IExifReader::TagType::Orientation);
        const int orientation = boost::any_cast<int>(orientation_raw);

        QImage rotated = thumbnail;
        switch(orientation)
        {
            case 0:
            case 1:
                break;    // nothing to do - no data, or normal orientation

            case 2:
                rotated = thumbnail.mirrored(true, false);
                break;

            case 3:
            {
                QTransform transform;
                transform.rotate(180);

                rotated = thumbnail.transformed(transform, Qt::SmoothTransformation);
                break;
            }

            case 4:
                rotated = thumbnail.mirrored(false, true);
                break;

            case 5:
            {
                QTransform transform;
                transform.rotate(270);

                rotated = thumbnail.mirrored(true, false).transformed(transform);
                break;
            }

            case 6:
            {
                QTransform transform;
                transform.rotate(90);

                rotated = thumbnail.transformed(transform, Qt::SmoothTransformation);
                break;
            }

            case 7:
            {
                QTransform transform;
                transform.rotate(90);

                rotated = thumbnail.mirrored(true, false).transformed(transform);
                break;
            }

            case 8:
            {
                QTransform transform;
                transform.rotate(270);

                rotated = thumbnail.transformed(transform);
                break;
            }
        }

        return rotated;
    }

    ThumbnailInfo m_info;
    ThumbnailGenerator::Callback m_callback;
    const ThumbnailGenerator* m_generator;
};


uint qHash(const ThumbnailInfo& key, uint seed = 0)
{
    return qHash(key.path) ^ qHash(key.height) ^ seed;
}


ThumbnailGenerator::ThumbnailGenerator():
    m_tasks(),
    m_executor(nullptr),
    m_photosManager(nullptr),
    m_logger(nullptr),
    m_exifReaderFactory(nullptr)
{

}


ThumbnailGenerator::~ThumbnailGenerator()
{

}


void ThumbnailGenerator::dismissPendingTasks()
{
    m_tasks->clear();
}


void ThumbnailGenerator::set(ITaskExecutor* executor)
{
    m_executor = executor;

    m_tasks = std::move( m_executor->getCustomTaskQueue() );
}


void ThumbnailGenerator::set(IPhotosManager* photosManager)
{
    m_photosManager = photosManager;
}


void ThumbnailGenerator::set(ILogger* logger)
{
    m_logger = logger;
}


void ThumbnailGenerator::set(IExifReaderFactory* exifFactory)
{
    m_exifReaderFactory = exifFactory;
}


void ThumbnailGenerator::generateThumbnail(const ThumbnailInfo& info, const Callback& callback) const
{
    auto task = std::make_unique<GenerationTask>(info, callback, this);
    m_tasks->push(std::move(task));
}


ThumbnailCache::ThumbnailCache():
    m_cacheMutex(),
    m_cache(2048)
{

}


ThumbnailCache::~ThumbnailCache()
{

}


void ThumbnailCache::add(const ThumbnailInfo& info, const QImage& img)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);

    QImage* new_img = new QImage(img);

    m_cache.insert(info, new_img);
}


boost::optional<QImage> ThumbnailCache::get(const ThumbnailInfo& info) const
{
    boost::optional<QImage> result;

    std::lock_guard<std::mutex> lock(m_cacheMutex);

    if (m_cache.contains(info))
        result = *m_cache[info];

    return result;
}
