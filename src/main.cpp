#include "main.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <chrono>
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

cmain::cmain(int argc, char **argv) : mArgs(argc, argv)
{
    uint16_t port = 8080;

    config_logging();

    if (!mArgs.validate({"d","p"}))
        showhelp();
    if (mArgs.numArgs() < 1)
        showhelp();

    if (mArgs.hasOpt({"p"}))
    {
        std::string portopt = mArgs.getValue({"p"});
        if (portopt.length()==0)
            showhelp();
        port = str2uint16(portopt);
        if (port<80)
            showhelp();
    }


    spdlog::info("------------------------------------------------------------------------");
    spdlog::info("tabfile running on port " + num2str(port) + "! Logging to " + getLogPath());

    if (mArgs.hasOpt({"d"}))
    {
        std::cout << ">> DAEMONISING " << std::endl
                  << std::flush;
        if (daemonise() != EXIT_SUCCESS)
            throw std::runtime_error("Failed to daemonise");
    }

    mDir = std::filesystem::canonical(mArgs.getArg(0));
    if (!std::filesystem::is_directory(mDir))
        throw std::runtime_error("No such directory: "+mDir.string());

    spdlog::info("reading from "+mDir.string());

    addroute_Root();
    addroute_Robots();
    addroute_Files();

    mApp.port(port)
        .multithreaded()
        .run();
}

cmain::~cmain()
{
}

void cmain::addroute_Files()
{
    CROW_ROUTE(mApp, "/<string>")
    ([&](std::string s) {
        // nlohmann::json data;
        // data["url"]="/";

        std::chrono::steady_clock::time_point tbegin = std::chrono::steady_clock::now();
        std::filesystem::path p;
        if (string2path(s,p))
        {
            std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now();
            spdlog::info("[{}] -> [{}] in {} µs",s,p.filename().string(),
                            std::chrono::duration_cast<std::chrono::microseconds>(tend - tbegin).count()); 
            
            crow::response res(200);
            res.set_static_file_info_unsafe(p.string());
            if (iequals(p.extension(),".mp4"))
                res.add_header("Content-Type","video/mp4");
            return res;
        }
        else
            return crow::response(404,R"END(
<html><body>
<center>
<p><h1>Tabula.Directory does not contain the specified file.</h1></p>

<p>For assistance, please contact our Support team:</p>
<a href="https://www.tabula.live/contact">Contact Us</a>
</center>
</body></html>
            )END");
    });
}

void cmain::addroute_Robots()
{
    CROW_ROUTE(mApp, "/robots.txt")
    ([&] {

        return crow::response(200,
        R"END(
User-agent: *
Disallow: /
        )END");
});
}

void cmain::addroute_Root()
{
    CROW_ROUTE(mApp, "/")
    ([&] {
             return crow::response(200,R"END(
<html><body>
<center>
<p><h1>The Tabula file directory is not browseable.</h1></p>

<p>For assistance, please contact our Support team:</p>
<a href="https://www.tabula.live/contact">Contact Us</a>
</center>
</body></html>
            )END");       
    });
}

void cmain::showhelp()
{
      std::cout <<
        R"END(

    tabfile DIRECTORY [OPTIONS]

    Options:
        -p=PORT
            run on given PORT (default is 8080)
        -d
            run as daemon
      
)END";
    exit(0);
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


namespace fs = std::filesystem;
bool cmain::string2path(const std::string s, std::filesystem::path &p)
{
    for (auto const& dir_entry : fs::recursive_directory_iterator(mDir))
    {
        if (dir_entry.is_directory())
        {
            std::string x = dir_entry.path().filename();
            if (matches(s,x))
            {
                bool foundlatest=false;
                std::string latestd="0";
                for (auto const & sub_entry : fs::directory_iterator(dir_entry.path()))
                {
                    std::string s = sub_entry.path().filename();
                    if (getdate(s)>getdate(latestd))
                    {
                        latestd=s.substr(0,8);
                        p=sub_entry.path();
                        foundlatest=true;
                    }
                }
                if (foundlatest)
                    return true;
            }
        }
    }

    return false;
}

bool cmain::matches(const std::string &search, const std::string &folder)
{
    // if (folder.length()<search.length())
    //     return false;

    return iequals(search,folder);
}
