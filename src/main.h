#ifndef __MAIN_H
#define __MAIN_H

#include <filesystem>

#include "args.h"


int main(int argc, char **argv);

class cmain
{
    public:
        cmain(int argc, char **argv);
        ~cmain();

    private:        
        void showhelp();
    
    private:
        cArgs mArgs;
        std::filesystem::path mDir;
        std::string mURL;
};


#endif