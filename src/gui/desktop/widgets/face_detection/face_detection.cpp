
#include <QQuickView>
#include <QEventLoop>

#include "face_detection.hpp"


FaceDetection::FaceDetection()
{
}


void FaceDetection::exec()
{
    QQuickView view(QUrl("qrc:/ui/Dialogs/FaceDetection.qml"));
    view.show();

    QEventLoop loop;
    QObject::connect(&view, &QQuickView::destroyed, &loop, &QEventLoop::quit);
    loop.exec();
}
