
#include <QQuickView>

#include "face_detection.hpp"


FaceDetection::FaceDetection()
{
}


void FaceDetection::exec()
{
    QQuickView view(QUrl("qrc:/ui/Dialogs/FaceDetection.qml"));
}
