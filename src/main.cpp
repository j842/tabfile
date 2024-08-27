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

int  daemonise()
{
    if (daemon(0,0)!=0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

std::string num2str(int n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}


cmain::cmain(int argc, char **argv)  : mArgs(argc,argv)
{
  const uint16_t port=8080;

  config_logging();

  spdlog::info("------------------------------------------------------------------------");
  spdlog::info("tabfile running on port "+num2str(port)+"! Logging to "+getLogPath());

    if (!mArgs.validate({"d"}))
        showhelp();

    if (mArgs.hasOpt({"d"}))
    {
        std::cout << ">> DAEMONISING " << std::endl
                  << std::flush;
        if (daemonise() != EXIT_SUCCESS)
            throw std::runtime_error("Failed to daemonise");
    }

  addroute_Main();

  mApp.port(port)
    .multithreaded()
    .run();

}

cmain::~cmain()
{
}

void cmain::addroute_Main()
{
    CROW_ROUTE(mApp, "/")
    ([&] {
        nlohmann::json data;
        data["url"]="/";
        return crow::response("<html><body>hi</body></html>");
    });
}

void cmain::showhelp()
{
      std::cout <<
        R"END(

    tabfile [OPTIONS]

    Options:
        -d
            run as daemon
      
)END";
    exit(0);
}
