#ifndef __SYMBRO_H
#define __SYMBRO_H

#include <string>
#include <filesystem>

class symbro
{
    public:
        symbro(std::filesystem::path p);

        void erase();

        void rescan();

        void watch(); // blocks forever!


    private:
        std::filesystem::path mDir;
        std::filesystem::path mSource;
        std::filesystem::path mOutput;
};


#endif