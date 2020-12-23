
#include "dlib_face_recognition_api.hpp"

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/dnn.h>
#include <dlib/cuda/cuda_dlib.h>
#include <QRgb>

#include <core/ilogger.hpp>
#include <core/lazy_ptr.hpp>
#include <system/filesystem.hpp>

#include "cnn_face_detector.hpp"
#include "face_recognition.hpp"

#ifdef DLIB_USE_CUDA
#define CUDA_AVAILABLE true
#else
#define CUDA_AVAILABLE false
#endif


namespace dlib_api
{
    namespace
    {
        constexpr char predictor_5_point_model[] = "shape_predictor_5_face_landmarks.dat";
        constexpr char predictor_68_point_model[] = "shape_predictor_68_face_landmarks.dat";
        constexpr char human_face_model[] = "mmod_human_face_detector.dat";
        constexpr char face_recognition_model[] = "dlib_face_recognition_resnet_model_v1.dat";

        int cuda_devices() 
        {
            // if cuda was disabled during dlib build then get_num_devices() will return 1 which is not what we want
            const static int devs = CUDA_AVAILABLE ? dlib::cuda::get_num_devices() : 0;

            return devs;
        }

        // helpers

        QString models_path()
        {
            const QString path = FileSystem().getDataPath() + "/face_recognition_models";

            return path;
        }

        template<typename T>
        T deserialize_from_file(const QString& file_name)
        {
            T object;
            dlib::deserialize(file_name.toStdString()) >> object;

            return object;
        }

        template<const char* name>
        QString modelPath()
        {
            const QString full_path = models_path() + "/" + name;

            return full_path;
        }

        template<typename T, const char* model>
        struct ObjectDeserializer
        {
            T operator()() const
            {
                const QString model_path = modelPath<model>();

                return deserialize_from_file<T>(model_path);
            }
        };

        dlib::matrix<dlib::rgb_pixel> qimage_to_dlib_matrix(const QImage& qimage)
        {
            dlib::matrix<dlib::rgb_pixel> matrix;

            const QRect size = qimage.rect();
            matrix.set_size(size.height(), size.width());

            for(int r = 0; r < size.height(); r++)
                for(int c = 0; c < size.width(); c++)
                {
                    const QRgb rgb = qimage.pixel(c, r);
                    matrix(r, c) = dlib::rgb_pixel(qRed(rgb), qGreen(rgb), qBlue(rgb));
                }

            return matrix;
        }

        QRect dlib_rect_to_qrect(const dlib::rectangle& rect)
        {
            const QRect qrect(rect.left(), rect.top(),
                              rect.right() - rect.left(), rect.bottom() - rect.top());

            return qrect;
        }

        template<typename DlibRect>
        QVector<QRect> dlib_rects_to_qrects(const std::vector<DlibRect>& dlib_rects)
        {
            QVector<QRect> qrects;
            for (const auto& rect: dlib_rects)
            {
                const QRect qrect = dlib_rect_to_qrect(rect);
                qrects.push_back(qrect);
            }

            return qrects;
        }

        cnn_face_detection_model_v1* construct_cnn_face_detector()
        {
            const auto cnn_face_detection_model = modelPath<human_face_model>();
            return new cnn_face_detection_model_v1(cnn_face_detection_model.toStdString());
        }
    }


    struct FaceLocator::Data
    {
        lazy_ptr<cnn_face_detection_model_v1, decltype(&construct_cnn_face_detector)> cnn_face_detector;
        lazy_ptr<dlib::frontal_face_detector, decltype(&dlib::get_frontal_face_detector)> hog_face_detector;
        std::unique_ptr<ILogger> logger;

        explicit Data(ILogger* l)
            : cnn_face_detector(&construct_cnn_face_detector)
            , hog_face_detector(&dlib::get_frontal_face_detector)
            , logger(l->subLogger("FaceLocator"))
        {

        }
    };


    FaceLocator::FaceLocator(ILogger* logger):
        m_data(std::make_unique<Data>(logger))
    {
        const int devices = cuda_devices();
        const bool cuda_available = devices > 0;

        if (cuda_available)
            m_data->logger->info(QString("%1 CUDA devices detected").arg(devices));
        else
            m_data->logger->warning("No CUDA devices");
    }


    FaceLocator::~FaceLocator()
    {

    }


    QVector<QRect> FaceLocator::face_locations(const QImage& qimage, int number_of_times_to_upsample)
    {
        std::optional<QVector<QRect>> faces;

        // Use cnn by default as it is fast and most accurate.
        // However it may fail (returned optional will be empty)
        m_data->logger->debug(QString("Looking for faces with cnn in image of size %1x%2")
                                .arg(qimage.width())
                                .arg(qimage.height()));

        faces = _face_locations_cnn(qimage, number_of_times_to_upsample);

        if (faces.has_value() == false)
        {
            m_data->logger->debug("Image too big for cnn, trying hog");
            faces = _face_locations_hog(qimage, number_of_times_to_upsample);

            // use faces found by hog to retry cnn search for more accurate results
            if (faces.has_value())
            {
                m_data->logger->debug(QString("Found %1 face(s). Trying cnn to improve faces positions").arg(faces->size()));

                for(QRect& face: faces.value())
                {
                    m_data->logger->debug(QString("Trying cnn for face %1,%2 (%3x%4)")
                                .arg(face.left())
                                .arg(face.top())
                                .arg(face.width())
                                .arg(face.height()));

                    auto cnn_faces = _face_locations_cnn(qimage, face);

                    if (cnn_faces.has_value() == false)
                        m_data->logger->debug("Face too big for cnn");
                    else if (cnn_faces->size() == 1)
                    {
                        // replace hog face with cnn face
                        face = cnn_faces->front();

                        m_data->logger->debug(QString("Improved face position to %1,%2 (%3x%4)")
                                .arg(face.left())
                                .arg(face.top())
                                .arg(face.width())
                                .arg(face.height()));
                    }
                }
            }
        }

        QVector<QRect> facesFound = faces.has_value()? faces.value(): QVector<QRect>();
        m_data->logger->debug(QString("Found %1 face(s)").arg(facesFound.size()));

        return facesFound;
    }


    QVector<QRect> FaceLocator::face_locations_cnn(const QImage& qimage, int number_of_times_to_upsample)
    {
        const auto dlib_results = m_data->cnn_face_detector->detect(qimage, number_of_times_to_upsample);
        const auto faces = dlib_rects_to_qrects(dlib_results);

        return faces;
    }


    QVector<QRect> FaceLocator::face_locations_hog(const QImage& qimage, int number_of_times_to_upsample)
    {
        dlib::matrix<dlib::rgb_pixel> image = qimage_to_dlib_matrix(qimage);

        const auto dlib_results = (*m_data->hog_face_detector)(image, number_of_times_to_upsample);
        const QVector<QRect> faces = dlib_rects_to_qrects(dlib_results);

        return faces;
    }


    std::optional<QVector<QRect>> FaceLocator::_face_locations_cnn(const QImage& qimage, int number_of_times_to_upsample)
    {
        std::optional<QVector<QRect>> faces;

        try
        {
            faces = face_locations_cnn(qimage, number_of_times_to_upsample);
        }
        catch(const dlib::cuda_error& err)
        {
            // image was too big for being processed
            // due to an issue in dlib, we just need to call face_locations_cnn here again
            QImage empty_image(10, 10, QImage::Format_Mono);
            try
            {
                face_locations_cnn(empty_image, 0);
            }
            catch(const dlib::cuda_error& err)
            {
                // we will end up here as long as https://github.com/davisking/dlib/issues/1984 exists
                // covered by learning tests
            }
        }

        return faces;
    }


    std::optional<QVector<QRect>> FaceLocator::_face_locations_cnn(const QImage& image, const QRect& rect)
    {
        // enlarge original rect by some margins so we won't miss face
        const int width = rect.width();
        const int height = rect.height();
        const int horizontalMargin = static_cast<int>(width * .2);
        const int verticalMargin = static_cast<int>(height * .2);

        const QPoint origin(rect.left() - horizontalMargin, rect.top() - verticalMargin);
        const QSize faceWithMarginsSize(width + horizontalMargin * 2, height + verticalMargin *2);
        const QRect rectWithMargins(origin, faceWithMarginsSize);
        const QImage imageWithMargins = image.copy(rectWithMargins);

        std::optional< QVector< QRect > > faces;
        for (int upsample = 0; upsample < 3; upsample++)
        {
            faces = _face_locations_cnn(imageWithMargins, upsample);

            if (faces.has_value())
            {
                // there can be 0 if cnn failed to find face(should not happend) or more than 1 (due to margins possibly)
                // at this moment only one face is being handled
                if (faces->size() == 1)
                {
                    faces->front().translate(origin);
                    break;
                }
                else
                    faces.reset();
            }
        }

        return faces;
    }


    std::optional<QVector<QRect>> FaceLocator::_face_locations_hog(const QImage& qimage, int number_of_times_to_upsample)
    {
        return face_locations_hog(qimage, number_of_times_to_upsample);
    }


    struct FaceEncoder::Data
    {
        Data()
            : face_encoder( modelPath<face_recognition_model>().toStdString() )
        {
        }

        face_recognition_model_v1 face_encoder;

        lazy_ptr<dlib::shape_predictor, ObjectDeserializer<dlib::shape_predictor, predictor_5_point_model>> predictor_5_point;
        lazy_ptr<dlib::shape_predictor, ObjectDeserializer<dlib::shape_predictor, predictor_68_point_model>> predictor_68_point;
    };


    FaceEncoder::FaceEncoder()
        : m_data(std::make_unique<Data>())
    {
    }


    FaceEncoder::~FaceEncoder()
    {

    }


    std::vector<double> FaceEncoder::face_encodings(const QImage& qimage, int num_jitters, EncodingsModel model)
    {
        // here we assume, that given image is a face extraceted from image with help of face_locations()
        const QSize size = qimage.size();
        const dlib::rectangle face_location(0, 0, size.width() - 1 , size.height() -1);
        const dlib::shape_predictor& pose_predictor = model == large?
                                                      *m_data->predictor_68_point :
                                                      *m_data->predictor_5_point;

        const auto image = qimage_to_dlib_matrix(qimage);
        const auto object_detection = pose_predictor(image, face_location);

        std::vector<double> result;

        try
        {
            const auto encodings = m_data->face_encoder.compute_face_descriptor(qimage, object_detection, num_jitters);
            result = std::vector<double>(encodings.begin(), encodings.end());
        }
        catch(const dlib::cuda_error& err)
        {
            std::cerr << err.what() << std::endl;
        }

        return result;
    }


    std::vector<bool> compare_faces(const std::vector<FaceEncodings>& known_face_encodings, const FaceEncodings& face_encoding_to_check, double tolerance)
    {
        const std::size_t faces = known_face_encodings.size();
        const std::vector distances = face_distance(known_face_encodings, face_encoding_to_check);

        std::vector<bool> results(faces, false);

        for(std::size_t i = 0; i < faces; i++)
            results[i] = distances[i] <= tolerance;

        return results;
    }


    std::vector<double> face_distance(const std::vector<FaceEncodings>& face_encodings, const FaceEncodings& face_to_compare)
    {
        std::vector<double> results;
        results.reserve(face_encodings.size());

        for(const FaceEncodings& face_encoding: face_encodings)
        {
            const std::size_t size = std::min(face_encoding.size(), face_to_compare.size());
            assert(size == 128);

            std::vector<double> diff(size, 0.0);

            for(std::size_t i = 0; i < size; i++)
                diff[i] = face_encoding[i] - face_to_compare[i];

            // calculating 2-norm from `diff` as in original python code
            // https://docs.scipy.org/doc/numpy/reference/generated/numpy.linalg.norm.html#numpy.linalg.norm
            // https://en.wikipedia.org/wiki/Norm_(mathematics)  -> p-norm

            const double norm_squared = std::accumulate(diff.begin(), diff.end(), 0.0, [](double sum, double v)
            {
                return sum + std::fabs(v) * std::fabs(v);
            });

            const double norm = std::sqrt(norm_squared);

            results.push_back(norm);
        }

        return results;
    }
}
