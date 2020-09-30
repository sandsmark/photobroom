
#include "faces_reader.hpp"


FacesReader::FacesReader(Database::IDatabase& db, ICoreFactoryAccessor& core)
    : m_db(db)
    , m_core(core)
{

}
