#include "main.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logging.h"


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

cmain::cmain(int argc, char **argv)  : mArgs(argc,argv)
{
  config_logging();

    spdlog::info("------------------------------------------------------------------------");
    spdlog::info("tabfile!");


}

cmain::~cmain()
{
}
