#include <spdlog/spdlog.h>


#include "symbro.h"
#include "utils.h"




symbro::symbro(std::filesystem::path p) :  
    mDir(p),
    mSource(p/"source"),
    mOutput(p/"output")
{
    if (!std::filesystem::is_directory(mSource))
        throw std::runtime_error("Missing required directory for source files: "+(mSource).string());

    if (!std::filesystem::is_directory(mOutput))
        throw std::runtime_error("Missing required directory for output files: "+(mOutput).string());
}

void symbro::erase()
{
    spdlog::info("- Deleting symlinks in {}",mOutput.string());
    deleteDirectoryContents(mOutput);
}

void symbro::rescan()
{
    std::chrono::steady_clock::time_point tbegin = std::chrono::steady_clock::now();

    spdlog::info("- Rescanning {}",mSource.string());

    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(mSource))
        if (dir_entry.is_directory())
        {
            std::string fname = dir_entry.path().filename();
            bool foundlatest=false;
            std::string latestd="0";
            std::filesystem::path p;
            for (auto const & sub_entry : std::filesystem::directory_iterator(dir_entry.path()))
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
            { // check symlink
                bool relink=true;
                std::filesystem::path lnk = mOutput/(fname+p.extension().string());
                if (std::filesystem::is_symlink(lnk))
                { // is it up to date?
                    std::filesystem::path orig = std::filesystem::read_symlink(lnk);
                    if (std::filesystem::equivalent(orig,p))
                        relink=false;
                }

                if (relink)
                {
                    if (std::filesystem::exists(lnk))
                        std::filesystem::remove(lnk);
                    
                    std::filesystem::create_symlink(p,lnk);

                    spdlog::info("-- Created symlink: {} -> {}",
                        (fname+p.extension().string())
                        ,p.string());
                }
            }
        }

    std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now();
    spdlog::info("- Rescan completed in {} ms",std::chrono::duration_cast<std::chrono::milliseconds>(tend - tbegin).count());
}

void symbro::watch()
{
    while (1)
    {
        watcher w({mSource});
        w.waitforchange();
        spdlog::info("- Source directory changed.");
        rescan();
    }
}

