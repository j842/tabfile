#include <sstream>
#include <sys/wait.h>
#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>

#include "utils.h"



std::string makecanonicalslash(const std::string &s)
{
    std::string ss(s);
    if (std::filesystem::exists(ss))
        ss = std::filesystem::canonical(ss);
    if (ss.back()!='/')
        ss.push_back('/');
    return ss;
}


watcher::watcher(std::vector<std::string> recursivepaths)
{
    std::string s="- Recursively Watching ";
    for (auto i : recursivepaths)    
        s+=i+" ";
    spdlog::info(s);

    //https://developer.ibm.com/tutorials/l-ubuntu-inotify/
    fd = inotify_init();
    if ( fd < 0 ) 
        throw std::runtime_error( "inotify_init" );
    for (auto p : recursivepaths)
    {
        std::string pc = makecanonicalslash(p);
        if (std::filesystem::exists(pc))
        {
            wd.push_back(
                inotify_add_watch( fd, pc.c_str(), 
                IN_CLOSE_WRITE  | IN_CREATE | IN_DELETE )
            );

            for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(pc))
                if (dir_entry.is_directory())
                    wd.push_back(
                        inotify_add_watch( fd, dir_entry.path().c_str(), 
                        IN_CLOSE_WRITE  | IN_CREATE | IN_DELETE )
                    );
        }
    }
}

watcher::~watcher()
{
    for (auto i : wd)
        inotify_rm_watch( fd, i );
    close( fd );    
}

void watcher::waitforchange() // create, modify, delete
{
    if (wd.size()>0)
        // block until one or more events arrive.
        /*length =*/ read( fd, buffer, BUF_LEN );  
}




std::string num2str(int n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

uint16_t str2uint16(const std::string & s)
{
    try
    {    
        int myInt(std::stoi(s));
        uint16_t myInt16(0);
        if (myInt <= static_cast<int>(UINT16_MAX) && myInt >=0) {
            myInt16 = static_cast<uint16_t>(myInt);
            return myInt16;
        }    
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}


int getdate(std::string s)
{
    try
    {
        {
            int d = stoi(s.substr(0,8));
            return d;
        }
    }
    catch(const std::exception& e)
    {
        return 0;
    }
}

void deleteDirectoryContents(const std::filesystem::path& dir)
{
    for (const auto& entry : std::filesystem::directory_iterator(dir)) 
        std::filesystem::remove_all(entry.path());
}
