
#ifndef FACEDETECTION_HPP
#define FACEDETECTION_HPP

#include <QStandardItemModel>
#include <database/idatabase.hpp>


class FaceDetection
{
    public:
        FaceDetection(Database::IDatabase &);

        void exec();

    private:
        QStandardItemModel m_unknownFaces;
        Database::IDatabase& m_db;
};

#endif
