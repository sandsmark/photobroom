
#include <QFileInfo>

#include <core/icore_factory_accessor.hpp>
#include <core/iexif_reader.hpp>
#include <core/itask_executor.hpp>
#include <core/oriented_image.hpp>
#include <core/task_executor_utils.hpp>
#include <database/ibackend.hpp>
#include <database/idatabase.hpp>
#include <face_recognition/face_recognition.hpp>

#include "faces_reader.hpp"


template<typename T>
struct ExecutorTraits<Database::IDatabase, T>
{
    static void exec(Database::IDatabase* db, T&& t)
    {
        db->exec(std::forward<T>(t));
    }
};


FacesReader::FacesReader(Database::IDatabase& db, ICoreFactoryAccessor& core)
    : m_core(core)
    , m_db(db)
{

}


FacesReader::~FacesReader()
{
    m_callback_ctrl.invalidate();
}


void FacesReader::get(const Photo::Id& id, const std::function<void (QVector<QRect>)>& callback)
{
    auto safe_callback = m_callback_ctrl.make_safe_callback([this, id, callback]{
        const auto faces = findFaces(id);
        callback(faces);
    });

    auto executor = m_core.getTaskExecutor();

    runOn(executor, safe_callback);
}


QVector<QRect> FacesReader::findFaces(const Photo::Id& id)
{
    QVector<QRect> result;

    const QString path = pathFor(id);
    const QFileInfo pathInfo(path);
    const QString full_path = pathInfo.absoluteFilePath();
    const OrientedImage image = OrientedImage(m_core.getExifReaderFactory()->get(), full_path);

    const std::vector<QRect> list_of_faces = fetchFacesFromDb(id);

    if (list_of_faces.empty())
    {
        FaceRecognition face_recognition(&m_core);
        const auto faces = face_recognition.fetchFaces(full_path);

        for(const QRect& face: faces)
            result.append(face);
    }
    else
    {
        result.reserve(static_cast<int>(list_of_faces.size()));

        std::copy(list_of_faces.cbegin(), list_of_faces.cend(), std::back_inserter(result));
    }

    return result;
}


QString FacesReader::pathFor(const Photo::Id& id) const
{
    return evaluate<QString(Database::IBackend &)>(&m_db, [id, db = &m_db](Database::IBackend &)
    {
        Database::IUtils& db_utils = db->utils();
        auto photo = db_utils.getPhotoFor(id);

        return photo->getPath();
    });
}


std::vector<QRect> FacesReader::fetchFacesFromDb(const Photo::Id& id) const
{
    return evaluate<std::vector<QRect>(Database::IBackend &)>
        (&m_db, [id](Database::IBackend& backend)
    {
        std::vector<QRect> faces;

        const auto people = backend.peopleInformationAccessor().listPeople(id);
        for(const auto& person: people)
            if (person.rect.isValid())
                faces.push_back(person.rect);

        return faces;
    });
}
