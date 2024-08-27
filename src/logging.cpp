#include <iostream>
#include <string>

#include "logging.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>

#include "crowinc.h"

const std::string getLogPath()
{
    static const std::string gLogPath = "/tmp/tabfile.log";

    return gLogPath;
}

class CustomLogger : public crow::ILogHandler {
 public:
  CustomLogger() {}
  void log(std::string message, crow::LogLevel level) {

    switch (level)
    {
        case crow::LogLevel::Warning:
            spdlog::warn("CROW: {}",message);
            break;        
        case crow::LogLevel::Error:
            spdlog::error("CROW: {}",message);
            break;        
        case crow::LogLevel::Critical:
            spdlog::critical("CROW: {}",message);
            break;        
        case crow::LogLevel::Info:
        case crow::LogLevel::Debug:
        default:
            spdlog::debug("CROW: {}",message);
            break;
    }
  }
};

static CustomLogger sLogger;

void config_logging()
{
        try 
        {
            std::vector<spdlog::sink_ptr> sinks;
            //sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(getConfig().getLogPath()));
            //sinks.push_back(std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(500));
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(getLogPath(),5 * 1024 * 1024,5));
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

            auto logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
            spdlog::register_logger(logger); //if it would be used in some other place
            spdlog::set_default_logger(logger);

            spdlog::set_level(spdlog::level::info);

            logger->flush_on(spdlog::level::info);
            //spdlog::flush_every (std::chrono::seconds(5));
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            std::cerr << "Log init failed: " << ex.what() << std::endl;
            exit(EXIT_FAILURE);
        }


    // configure logger for crow.
    crow::logger::setHandler(&sLogger);
}

std::string get_ringbuffer()
{
    auto logger = spdlog::get("main");
    if (!logger) return "ERROR - COULDNT GET LOGGER";
    auto const &sinks_tmp = std::dynamic_pointer_cast<spdlog::sinks::ringbuffer_sink_mt>(*(logger->sinks().begin()));
    auto vecstr = sinks_tmp->last_formatted();
    std::ostringstream imploded;
    std::copy(vecstr.begin(), vecstr.end(),
           std::ostream_iterator<std::string>(imploded));
    return imploded.str();
}
