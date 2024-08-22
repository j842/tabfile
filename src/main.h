#ifndef __MAIN_H
#define __MAIN_H


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
        
    private:
        crow::SimpleApp mApp;
        cArgs mArgs;
};


#endif