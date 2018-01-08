/*
 * tool for generating gif file from many images
 * Copyright (C) 2017  Michał Walenciak <Kicer86@gmail.com>
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

#ifndef ANIMATION_GENERATOR_HPP
#define ANIMATION_GENERATOR_HPP

#include <QObject>
#include <QSize>

#include <core/itask_executor.hpp>

class QProcess;

struct ILogger;

class AnimationGenerator: public QObject, public ITaskExecutor::ITask
{
        Q_OBJECT

    public:
        struct Data
        {
            QString convertPath;
            QString alignImageStackPath;
            QStringList photos;
            double fps;
            double delay;
            double scale;
            bool stabilize;

            Data(): convertPath(), alignImageStackPath(), photos(), fps(0.0), delay(0.0), scale(0.0), stabilize(false) {}
        };

        AnimationGenerator(const Data& data, ILogger *);
        AnimationGenerator(const AnimationGenerator &) = delete;
        ~AnimationGenerator();

        AnimationGenerator& operator=(const AnimationGenerator &) = delete;

        std::string name() const override;
        void perform() override;

        void cancel();

    signals:
        void operation(const QString &);
        void progress(int);
        void finished(const QString &);

    private:
        Data m_data;
        std::mutex m_cancelMutex;
        ILogger* m_logger;
        bool m_cancel;

        QStringList stabilize(const QString &);
        QString generateGif(const QStringList &);

        void startAndWaitForFinish(QProcess &);

    // internal signals:
    signals:
        void canceled();
};


#endif // ANIMATION_GENERATOR_HPP
