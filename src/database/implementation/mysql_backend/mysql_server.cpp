/*
 * A class for managing mysql server process
 * Copyright (C) 2013  Michał Walenciak <MichalWalenciak@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mysql_server.hpp"

#include <QProcess>
#include <QFile>
#include <QDir>

#include <boost/optional.hpp>

#include "configuration/configurationfactory.hpp"
#include "configuration/iconfiguration.hpp"
#include "configuration/entrydata.hpp"
#include "system/system.hpp"

#include "databasebuilder.hpp"

namespace
{
    const char* MySQL_daemon = "Database::Backend::MySQL::Server";

    const char* MySQL_config =
    "#Config based on Akonadi's config file                                                        \n"
    "[mysqld]                                                                                     \n"

    "# strict query parsing/interpretation                                                        \n"
    "# TODO: make Akonadi work with those settings enabled                                        \n"
    "# sql_mode=strict_trans_tables,strict_all_tables,strict_error_for_division_by_zero,no_auto_create_user,no_auto_value_on_zero,no_engine_substitution,no_zero_date,no_zero_in_date,only_full_group_by,pipes_as_concat\n"
    "# sql_mode=strict_trans_tables                                                               \n"

    "# DEBUGGING:\n"
    "# log all queries, useful for debugging but generates an enormous amount of data\n"
    "# log=mysql.full\n"
    "# log queries slower than n seconds, log file name relative to datadir (for debugging only)\n"
    "# log_slow_queries=mysql.slow\n"
    "# long_query_time=1\n"
    "# log queries not using indices, debug only, disable for production use\n"
    "# log_queries_not_using_indexes=1\n"
    "#\n"
    "# mesure database size and adjust innodb_buffer_pool_size\n"
    "# SELECT sum(data_length) as bla, sum(index_length) as blub FROM information_schema.tables WHERE table_schema not in (\"mysql\", \"information_schema\");\n"

    "# NOTES:\n"
    "# Keep Innob_log_waits and keep Innodb_buffer_pool_wait_free small (see show global status like \"inno%\", show global variables)\n"

    "#expire_logs_days=3\n"

    "#sync_bin_log=0\n"

    "# Use UTF-8 encoding for tables\n"
    "character_set_server=utf8\n"
    "collation_server=utf8_general_ci\n"

    "# use InnoDB for transactions and better crash recovery\n"
    "default_storage_engine=innodb\n"

    "# memory pool InnoDB uses to store data dictionary information and other internal data structures (default:1M)\n"
    "# Deprecated in MySQL >= 5.6.3\n"
    "innodb_additional_mem_pool_size=1M\n"

    "# memory buffer InnoDB uses to cache data and indexes of its tables (default:128M)\n"
    "# Larger values means less I/O\n"
    "innodb_buffer_pool_size=80M\n"

    "# Create a .ibd file for each table (default:0)\n"
    "innodb_file_per_table=1\n"

    "# Write out the log buffer to the log file at each commit (default:1)\n"
    "innodb_flush_log_at_trx_commit=2\n"

    "# Buffer size used to write to the log files on disk (default:1M for builtin, 8M for plugin)\n"
    "# larger values means less I/O\n"
    "innodb_log_buffer_size=1M\n"

    "# Size of each log file in a log group (default:5M) larger means less I/O but more time for recovery.\n"
    "innodb_log_file_size=64M\n"

    "# # error log file name, relative to datadir (default:hostname.err)\n"
    "log_error=mysql.err\n"

    "# print warnings and connection errors (default:1)\n"
    "log_warnings=2\n"

    "# Convert table named to lowercase\n"
    "lower_case_table_names=1\n"

    "# Maximum size of one packet or any generated/intermediate string. (default:1M)\n"
    "max_allowed_packet=32M\n"

    "# Maximum simultaneous connections allowed (default:100)\n"
    "max_connections=256\n"

    "# The two options below make no sense with prepared statements and/or transactions\n"
    "# (make sense when having the same query multiple times)\n"

    "# Memory allocated for caching query results (default:0 (disabled))\n"
    "query_cache_size=0\n"

    "# Do not cache results (default:1)\n"
    "query_cache_type=0\n"

    "# Do not use the privileges mechanisms\n"
    "skip_grant_tables\n"

    "# Do not listen for TCP/IP connections at all\n"
    "skip_networking\n"

    "# The number of open tables for all threads. (default:64)\n"
    "table_open_cache=200\n"

    "# How many threads the server should cache for reuse (default:0)\n"
    "thread_cache_size=3\n"

    "# wait 365d before dropping the DB connection (default:8h)\n"
    "wait_timeout=31536000\n"

    "[client]\n"
    "default-character-set=utf8\n";

    struct MySqlServerInit: public Configuration::IInitializer
    {

        MySqlServerInit()
        {
            std::shared_ptr<IConfiguration> config = ConfigurationFactory::get();

            config->registerInitializer(this);
        }

        virtual std::string getXml()
        {
            const std::string configuration_xml =
                "<configuration>                                        "
                "    <keys>                                             "
                "        <key name='" + std::string(MySQL_daemon) + "' />            "
                "    </keys>                                            "
                "</configuration>                                       ";

            return configuration_xml;
        }

    } init;
}


MySqlServer::MySqlServer(): m_serverProcess(new QProcess)
{

}


MySqlServer::~MySqlServer()
{
    std::cout << "MySQL Database Backend: closing down MySQL server" << std::endl;

    m_serverProcess->terminate();
    m_serverProcess->waitForFinished();

    std::cout << "MySQL Database Backend: MySQL server down" << std::endl;
}


QString MySqlServer::run_server(const QString& basePath)
{
    bool status = false;
    QString result = "";
    const std::string path = getDaemonPath();

    if (path.empty() == false)
    {
        const QString configFile = basePath + "mysql.conf";
        const QString baseDataPath = basePath + "db_data";
        const QString socketPath = basePath + "mysql.socket";

        status = createConfig(configFile);

        if (status)
        {
            const QString mysql_config  = "--defaults-file=" + configFile;
            const QString mysql_datadir = "--datadir=" + baseDataPath;
            const QString mysql_socket  = "--socket=" + socketPath;

            status = true;
            if (QDir(baseDataPath).exists() == false)
                status = initDB(baseDataPath.toStdString(), mysql_config.toStdString());

            if (status)
            {
                QStringList args = { mysql_config, mysql_datadir, mysql_socket};

                m_serverProcess->setProgram(path.c_str());
                m_serverProcess->setArguments(args);
                m_serverProcess->closeWriteChannel();

                std::cout << "MySQL Database Backend: " << path << " " << args.join(" ").toStdString() << std::endl;

                m_serverProcess->start();

                status = m_serverProcess->waitForStarted();

                result = status? socketPath : "";
            }
        }
    }

    return result;
}


std::string MySqlServer::getDaemonPath() const
{
    //get path to server
    std::shared_ptr<IConfiguration> config = ConfigurationFactory::get();

    boost::optional<Configuration::EntryData> daemonPath = config->findEntry(MySQL_daemon);

    std::string path;
    if (daemonPath)
        path = daemonPath->value();
    else
    {
        path = System::findProgram("mysqld");

        Configuration::EntryData mysqldPath(MySQL_daemon, path);
        config->addEntry(mysqldPath);
    }

    return path;
}


bool MySqlServer::initDB(const std::string& dbDir, const std::string& extraOptions) const
{
    const std::string path = System::findProgram("mysql_install_db");
    bool status = false;

    if (path.empty() == false)
    {
        QProcess init;

        const std::string userName = System::userName();
        const std::string dataDirOption  = "--datadir=" + dbDir;
        const std::string userNameOption = "--user=" + userName;

        init.start( path.c_str(), {dataDirOption.c_str(), userNameOption.c_str(), extraOptions.c_str()} );
        status = init.waitForStarted();
        init.waitForFinished();

        status = init.exitCode() == QProcess::NormalExit;

        if (!status)
        {
            std::cerr << "MySQL Database Backend: database initialization failed:" << std::endl;
            std::cerr << QString(init.readAll()).toStdString() << std::endl << std::endl;

            status = QDir(dbDir.c_str()).rmdir(dbDir.c_str());
        }
    }

    return status;
}


bool MySqlServer::createConfig(const QString& configFile) const
{
    bool status = true;

    if (QFile::exists(configFile) == false)
    {
        QFile os(configFile);

        status = os.open(QFile::WriteOnly);

        if (status)
            status = os.write(MySQL_config) != -1;
    }

    return status;
}
