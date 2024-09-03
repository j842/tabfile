#ifndef __SYMBRO_H
#define __SYMBRO_H

#include <string>
#include <filesystem>

class symbro
{
    public:
        symbro(std::filesystem::path p, std::string baseURL);

        void erase();
        void rescan();
        void make_index();

        void watch(); // blocks forever!

    private:
        void checklink(std::filesystem::path linktarget, std::filesystem::path lnk);

    private:
        std::filesystem::path mDir;
        std::filesystem::path mSource;
        std::filesystem::path mOutput;

        std::string mURL;
};


#endif