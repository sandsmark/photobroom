
#ifndef FACESREADER_HPP
#define FACESREADER_HPP

#include <core/function_wrappers.hpp>
#include <database/photo_data.hpp>

namespace Database
{
    struct IDatabase;
}

struct ICoreFactoryAccessor;


class FacesReader
{
    public:
        FacesReader(Database::IDatabase &, ICoreFactoryAccessor &);
        ~FacesReader();

        void get(const Photo::Id &, const std::function<void(QVector<QRect>)> &);

    private:
        safe_callback_ctrl m_callback_ctrl;
        ICoreFactoryAccessor& m_core;
        Database::IDatabase& m_db;

        QVector<QRect> findFaces(const Photo::Id &);
        QString pathFor(const Photo::Id &) const;
        std::vector<QRect> fetchFacesFromDb(const Photo::Id &) const;
};

#endif
