/*
    Database for photos
    Copyright (C) 2013  Michał Walenciak <MichalWalenciak@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "memorydatabase.hpp"

#include <assert.h>

#include <unordered_map>
#include <string>
#include <deque>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <boost/crc.hpp>
#include <boost/optional.hpp>

#include <OpenLibrary/palgorithm/ts_queue.hpp>

#include "entry.hpp"
#include "ifs.hpp"

namespace Database
{    
    namespace
    {
        void trampoline(MemoryDatabase::Impl *);
    }

    struct MemoryDatabase::Impl
    {
        Impl(Database::IConfiguration *config, const std::shared_ptr<FS> &stream): 
            m_db(), 
            m_configuration(config),
            m_stream(stream),
            m_backend(nullptr),
            m_backendMutex(),
            m_backendSet(),
			m_updateQueue(m_max_queue_len),
            m_storekeeperWork(true),
            m_storekeeper(trampoline, this)
        {
            
        }
        

        virtual ~Impl()
        {
            m_storekeeperWork = false;  //quit thread
            m_updateQueue.break_popping();
            
            assert(m_storekeeper.joinable());
            m_storekeeper.join();       //wait for quit
        }
        

        Impl(const MemoryDatabase::Impl &);

        
        Impl& operator=(const Impl &)
        {
            return *this;
        }        
             

        void add(const std::string &path, const IFrontend::Description &description)
        {
            (void) description;
            Entry entry;

            entry.m_d->m_crc = calcCrc(path);
            entry.m_d->m_path = decoratePath(path);

            m_db[entry.m_d->m_crc] = entry;
            
            registerUpdate(entry.m_d->m_crc);
        }

        
        std::string decoratePath(const std::string &path) const
        {
            return std::string("file://") + path;
        }
        
        
        void setBackend(const std::shared_ptr<IBackend> &b)
        {
            m_backend = b;
            
            if (m_backend != nullptr)
                m_backendSet.notify_all();
        }
        
        
        void storekeeper()      //storekeeper thread
        {
            while (m_storekeeperWork)        //as long as we are forced to work...
            {
                do
                {
                    std::unique_lock<std::mutex> lock(m_backendMutex);
                    
                    while(m_backend == nullptr)
                        m_backendSet.wait(lock, [&]{ return m_backend != nullptr; } );      //wait for signal if no backend
                    
                    boost::optional<Entry::crc32> entry = getItemToUpdate();
                    
                    if (entry)
                    {
                        const Entry &dbEntry = m_db[*entry];
                        m_backend->store(dbEntry);
                    }
                    else
                        assert(m_storekeeperWork == false);       //the only reason for empty queue is that we are quiting
                }
                while(m_updateQueue.empty() == false);  //do not back to main loop as long as there some data to be stored
            }
        }

        private:
            const static int m_max_queue_len = 256;                 //max len of db queue
            std::unordered_map<Entry::crc32, Entry> m_db;           //files managed by database
            Database::IConfiguration *m_configuration;
            std::shared_ptr<FS> m_stream;
            std::shared_ptr<IBackend> m_backend;
            std::mutex m_backendMutex;
            std::condition_variable m_backendSet;
            TS_Queue<std::deque<Entry::crc32>> m_updateQueue;       //entries to be stored in backend
            bool m_storekeeperWork;
            std::thread m_storekeeper;
            
            void registerUpdate(const Entry::crc32 &item)
            {                
                m_updateQueue.push_back(item);
            }
            
            boost::optional<Entry::crc32> getItemToUpdate()            
            {
                const boost::optional<Entry::crc32> entry = m_updateQueue.pop_front();
                
                return entry;
            }
            
            Entry::crc32 calcCrc(const std::string &path) const
            {
                const int MAX_SIZE = 65536;
                boost::crc_32_type crc;
                std::iostream *input = m_stream->openStream(path, std::ios_base::in | std::ios_base::binary);

                if (input != nullptr)
                    do
                    {
                        char buf[MAX_SIZE];

                        input->read(buf, MAX_SIZE);
                        crc.process_bytes(buf, input->gcount());
                    }
                    while(input->fail() == false);
                    
                const Entry::crc32 sum = crc();
                
                m_stream->closeStream(input);
                
                return sum;
            }
    };
    
    namespace
    {
        void trampoline(MemoryDatabase::Impl *impl)
        {
            impl->storekeeper();
        }
    }


    MemoryDatabase::MemoryDatabase(Database::IConfiguration *config, const std::shared_ptr<FS> &stream): m_impl(new Impl(config, stream) )
    {

    }


    MemoryDatabase::~MemoryDatabase()
    {

    }


    bool MemoryDatabase::addFile(const std::string &path, const IFrontend::Description &desc)
    {
		m_impl->add(path, desc);

        return true;
    }
    
    
    void MemoryDatabase::setBackend(const std::shared_ptr<Database::IBackend> &backend)
    {
        m_impl->setBackend(backend);
    }
    
}
