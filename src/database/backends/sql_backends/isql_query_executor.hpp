
#ifndef ISQLQUERYEXECUTOR_HPP
#define ISQLQUERYEXECUTOR_HPP


#include "database/database_status.hpp"


class QString;
class QSqlQuery;


namespace Database
{
    class SqlQuery;

    struct ISqlQueryExecutor
    {
        virtual ~ISqlQueryExecutor() {}

        virtual BackendStatus exec(const QString& query, QSqlQuery* result) const = 0;
        virtual BackendStatus exec(const SqlQuery& query, QSqlQuery* result) const = 0;
    };
}


#endif // ISQLQUERYEXECUTOR_HPP
