#include "main.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <chrono>
#include <iostream>

#include "logging.h"
#include "utils.h"
#include "symbro.h"

int main(int argc, char **argv)
{
    int rval = 0;

    try 
    {
        cmain __main(argc,argv);
    }
    catch (const std::runtime_error &e)
    {
        spdlog::error("Runtime Error: {}", e.what());
        rval = -1;
    }

    std::shared_ptr<spdlog::logger> logger = spdlog::get("main");
    if (logger)
        logger->flush();
    else 
        std::cerr << "Couldn't access logger." << std::endl;

    return rval;
}

int  daemonise()
{
    if (daemon(0,0)!=0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}



cmain::cmain(int argc, char **argv) : mArgs(argc, argv)
{
    config_logging();

    if (!mArgs.validate({"d"}))
        showhelp();
    if (mArgs.numArgs() < 1)
        showhelp();

    spdlog::info("------------------------------------------------------------------------");
    spdlog::info("tabfile running! Logging to " + getLogPath());

    mDir = std::filesystem::canonical(mArgs.getArg(0));
    if (!std::filesystem::is_directory(mDir))
        throw std::runtime_error("No such directory: "+mDir.string());

    spdlog::info("reading from "+mDir.string());

    if (mArgs.hasOpt({"d"}))
    {
        std::cout << ">> DAEMONISING " << std::endl
                  << std::flush;
        if (daemonise() != EXIT_SUCCESS)
            throw std::runtime_error("Failed to daemonise");
    }

    symbro b(mDir);
    b.erase();
    b.rescan();
    b.watch();
}

cmain::~cmain()
{
}

void cmain::showhelp()
{
      std::cout <<
        R"END(

    tabfile DIRECTORY [OPTIONS]

    Options:
        -d
            run as daemon
      
)END";
    exit(0);
}


