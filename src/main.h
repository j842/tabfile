#ifndef __MAIN_H
#define __MAIN_H

#include <filesystem>

#include "crowinc.h"
#include "json.hpp"
#include "args.h"


int main(int argc, char **argv);

class cmain
{
    public:
        cmain(int argc, char **argv);
        ~cmain();

    private:        
        void addroute_Main();

        void showhelp();
        
        bool string2path(const std::string s, std::filesystem::path & p);
        bool matches(const std::string & search, const std::string & folder);

    private:
        crow::SimpleApp mApp;
        cArgs mArgs;

        std::filesystem::path mDir;
};


#endif