
#ifndef DATABASE_SQLITE_BACKEND_HPP
#define DATABASE_SQLITE_BACKEND_HPP

#include <memory>

#include <QObject>
#include <QtPlugin>

#include <database/idatabase.hpp>
#include <database/backends/sql_backends/sql_backend.hpp>
#include <database/backends/sql_backends/generic_sql_query_generator.hpp>
#include <database/implementation/ibackend_qt_interface.hpp>

#include "database_sqlite_backend_export.h"

namespace Database
{

    class SQLiteBackend final: public ASqlBackend, GenericSqlQueryConstructor
    {
        public:
            SQLiteBackend();
            virtual ~SQLiteBackend();

        private:
            // ASqlBackend:
            virtual BackendStatus prepareDB(const ProjectInfo &) override;
            virtual bool dbOpened() override;
            virtual const IGenericSqlQueryGenerator* getGenericQueryGenerator() const override;
            virtual void set(IConfiguration *);

            //ISqlQueryConstructor:
            virtual QString prepareFindTableQuery(const QString &) const override;
            virtual QString getTypeFor(ColDefinition::Purpose) const;

            struct Data;
            std::unique_ptr<Data> m_data;
    };


    class DATABASE_SQLITE_BACKEND_EXPORT SQLitePlugin final: public IPlugin
    {
            Q_OBJECT
            Q_PLUGIN_METADATA(IID DatabasePluginInterface_iid FILE "sqlite_backend.json")
            Q_INTERFACES(Database::IPlugin)   //'Database' namespace is obligatory

        public:
            SQLitePlugin();
            virtual ~SQLitePlugin();

            virtual std::unique_ptr<IBackend> constructBackend() override;
            virtual QString backendName() const override;
            virtual ProjectInfo initPrjDir(const QString& dir, const QString& name) const override;
            virtual QLayout* buildDBOptions() override;
            virtual char simplicity() const override;
    };

}

#endif
