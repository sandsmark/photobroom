
#include <QQuickView>
#include <QEventLoop>

#include <database/iphoto_operator.hpp>
#include "face_detection.hpp"
#include "quick_views/qml_utils.hpp"


FaceDetection::FaceDetection(Database::IDatabase& db)
    : m_db(db)
{
    m_db.exec([](Database::IBackend& backend) {
        backend.photoOperator().getPhotos({});
    });
}


void FaceDetection::exec()
{
    QQuickView view;
    view.setSource(QUrl("qrc:/ui/Dialogs/FaceDetection.qml"));
    view.show();

    QEventLoop loop;
    QObject::connect(&view, &QQuickView::destroyed, &loop, &QEventLoop::quit);
    loop.exec();
}
