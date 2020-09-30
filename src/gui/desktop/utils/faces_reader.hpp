
#ifndef FACESREADER_HPP
#define FACESREADER_HPP

namespace Database
{
    struct IDatabase;
}

struct ICoreFactoryAccessor;


class FacesReader
{
    public:
        FacesReader(Database::IDatabase &, ICoreFactoryAccessor &);


    private:
        ICoreFactoryAccessor& m_core;
        Database::IDatabase& m_db;
};

#endif
