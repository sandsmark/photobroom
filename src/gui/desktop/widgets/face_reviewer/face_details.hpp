/*
 * Widget with details about person
 * Copyright (C) 2018  Michał Walenciak <Kicer86@gmail.com>
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
 */

#ifndef FACEDETAILS_HPP
#define FACEDETAILS_HPP

#include <QGroupBox>

class QLabel;

class FaceDetails: public QGroupBox
{
        Q_OBJECT

    public:
        FaceDetails(const QString &, QWidget *);

    public slots:
        void setOccurrences(int);
        void setModelPhoto(const QPixmap &);
        void setModelPhoto(const QImage &);

    private:
        QLabel* m_photo;
        QLabel* m_occurences;
};

#endif // FACEDETAILS_HPP
