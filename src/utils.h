#ifndef __UTILS_H
#define __UTILS_H

#include <string>
#include <sys/types.h>
#include <sys/inotify.h>
#include <vector>


std::string num2str(int n);
uint16_t str2uint16(const std::string & s);
int getdate(std::string s);
void deleteDirectoryContents(const std::filesystem::path& dir);

// watch a folder for changes in it.
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
class watcher
{
    public:
        watcher(std::vector<std::string> recursivepaths);
        ~watcher();

        void waitforchange(); // create, modify, delete

    private:
        std::vector<int> wd;
        int fd;
        char buffer[BUF_LEN];
};
    




#endif