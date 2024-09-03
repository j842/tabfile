#include <spdlog/spdlog.h>

#include "xlsxwriter.h"

#include "symbro.h"
#include "utils.h"




symbro::symbro(std::filesystem::path p, std::string baseURL) :  
    mDir(p),
    mSource(p/"source"),
    mOutput(p/"output"),
    mURL(baseURL)
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
                checklink(
                    p,
                    mOutput/(fname+p.extension().string())
                );
        }

    std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now();
    spdlog::info("- Rescan completed in {} ms",std::chrono::duration_cast<std::chrono::milliseconds>(tend - tbegin).count());
}

void symbro::make_index()
{
    std::filesystem::path indexpath = mSource/"__directory_index"/"index.xlsx";
    create_directories(indexpath.parent_path());

    spdlog::info("- Creating index at {}",indexpath.string());

    lxw_workbook  * workbook = workbook_new(indexpath.string().c_str());
    lxw_worksheet * worksheet = workbook_add_worksheet(workbook, "Directory Links");

    worksheet_set_column(worksheet,0,1,50,nullptr);
    worksheet_set_column(worksheet,2,2,80,nullptr);

    lxw_format *bold = workbook_add_format(workbook);
    format_set_bold(bold);
    format_set_bottom_color(bold,LXW_COLOR_BLACK );

    worksheet_write_string(worksheet, 0, 0, "Link", bold);
    worksheet_write_string(worksheet, 0, 1, "Parent", bold);
    worksheet_write_string(worksheet, 0, 2, "Original File", bold);


    int i=0;
    for (auto const & entry : std::filesystem::directory_iterator(mOutput))
    {
        i++;
        if (std::filesystem::is_symlink(entry.path()))
        {
            std::string url = mURL+"/"+entry.path().filename().string();
        	worksheet_write_string(worksheet, i, 0, url.c_str(), nullptr);

            std::filesystem::path sourcepath = std::filesystem::read_symlink(entry);
            std::string parent = sourcepath.lexically_relative(mSource).parent_path().parent_path();
            worksheet_write_string(worksheet, i, 1, parent.c_str(), nullptr);

            std::string origfile = sourcepath.filename();
            worksheet_write_string(worksheet, i, 2, origfile.c_str(), nullptr);
        }
    }
    workbook_close(workbook);

    checklink(indexpath, mOutput/"__directory_index.xlsx");
}

void symbro::watch()
{
    while (1)
    {
        watcher w({mSource});
        w.waitforchange();
        spdlog::info("- Source directory changed.");

        rescan();
        make_index();
    }
}

void symbro::checklink(std::filesystem::path linktarget, std::filesystem::path lnk)
{
    if (std::filesystem::is_symlink(lnk))
    { // is it up to date?
        std::filesystem::path orig = std::filesystem::read_symlink(lnk);
        if (std::filesystem::equivalent(orig,linktarget))
            return;
    }

    if (std::filesystem::exists(lnk))
        std::filesystem::remove(lnk);
        
    std::filesystem::create_symlink(linktarget,lnk);

    spdlog::info("-- Created symlink: {} -> {}",
        lnk.filename().string(), linktarget.filename().string());
}
