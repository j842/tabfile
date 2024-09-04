#include <spdlog/spdlog.h>

#include "symbro.h"
#include "utils.h"

#include "QrToPng.h"


symbro::symbro(std::filesystem::path p, std::string baseURL) :  
    mDir(p),
    mSource(p/"source"),
    mOutput(p/"output"),
    mLinks(p/"output"/"links"),
    mQR(p/"output"/"qr"),
    mURL(baseURL)
{
    if (!std::filesystem::is_directory(mSource))
        throw std::runtime_error("Missing required directory for source files: "+(mSource).string());

    create_directories(mLinks);    
    create_directories(mQR);    
}

// void symbro::erase()
// {
//     spdlog::info("- Deleting symlinks in {}",mLinks.string());
//     deleteDirectoryContents(mLinks);
// }

int symbro::rescan()
{
    int changedFiles=0;
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
                if (!iequals(sub_entry.path().extension(),".identifier")) // skip identifier files from Windows.
                {
                    std::string s = sub_entry.path().filename();
                    if (getdate(s)>getdate(latestd))
                    {
                        latestd=s.substr(0,8);
                        p=sub_entry.path();
                        foundlatest=true;
                    }
                }
            }

            if (foundlatest)
            {
                std::filesystem::path lnk = mLinks/(fname+p.extension().string());
                bool newlink = checklink(p,lnk);
                bool newqr = checkqr(lnk);

                if (newlink || newqr)
                    ++changedFiles;
            }
        }

    // check all links still valid
    for (auto const & entry : std::filesystem::directory_iterator(mLinks))
        if (std::filesystem::is_symlink(entry.path()))
            if (!std::filesystem::exists(entry.path()))
                {
                    spdlog::info("-- Removing bad link: {}",entry.path().filename().string());
                    std::filesystem::remove(entry.path());
                    ++changedFiles;
                }


    std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now();
    spdlog::info("- Rescan completed, {} changed files in {} ms",changedFiles,std::chrono::duration_cast<std::chrono::milliseconds>(tend - tbegin).count());
    return changedFiles;
}

void replaceunderscores(std::string & s)
{
    std::replace( s.begin(), s.end(), '_', ' '); 
}

void symbro::make_index()
{
    spdlog::info("- Creating index at {}.[html,xlsx]",(mOutput/"directory_index").string());

    indexwriter_xlsx xlsx(mOutput/"directory_index.xlsx");
    indexwriter_html html(mOutput/"directory_index.html");
    std::vector<indexwriter *> writers = {&xlsx,&html}; // just a vector of pointers. Yawn.

    int i=0;
    for (auto const & entry : std::filesystem::directory_iterator(mLinks))
    {
        if (std::filesystem::is_symlink(entry.path()))
        {
            i++;
            std::string url = getURL(entry.path());
            std::filesystem::path sourcepath = std::filesystem::read_symlink(entry);
            std::string parent = sourcepath.lexically_relative(mSource).parent_path().parent_path();
            std::string origfile = sourcepath.filename();
            std::string qrcodelink = getQRURL(entry.path());
            std::string title = sourcepath.parent_path().filename();
            replaceunderscores(title);

            for (auto & w : writers)
                w->addrow(i,url,parent,origfile,qrcodelink,title);
        }
    }
}

void symbro::watch()
{
    while (1)
    {
        watcher w({mSource});
        w.waitforchange();
        spdlog::info("- Source directory changed.");

        if (rescan()>0)
            make_index();
    }
}

// returns true if changed.
bool symbro::checklink(std::filesystem::path linktarget, std::filesystem::path lnk)
{
    if (std::filesystem::is_symlink(lnk))
    { // is it up to date?
        std::filesystem::path orig = std::filesystem::read_symlink(lnk);
        if (std::filesystem::exists(orig))
            if (std::filesystem::equivalent(orig,linktarget))
                return false;
    }

    if (std::filesystem::is_symlink(lnk) || std::filesystem::exists(lnk))
        std::filesystem::remove(lnk);
        
    std::filesystem::create_symlink(linktarget,lnk);

    spdlog::info("-- Created symlink: {} -> {}",
        lnk.filename().string(), linktarget.filename().string());

    return true;
}

bool symbro::checkqr(std::filesystem::path lnk)
{
    std::filesystem::path qr = getQRPath(lnk);
    
    if (std::filesystem::is_regular_file(qr))
        return false;

    std::string url = getURL(lnk);
    
    auto exampleQrPng1 = QrToPng(qr.string(), 300, 3, url, true, qrcodegen::QrCode::Ecc::MEDIUM);
    exampleQrPng1.writeToPNG();
    return true;
}

std::string symbro::getURL(std::filesystem::path lnk)
{
    std::string url = mURL+"/"+lnk.filename().string();
    return url;
}

const std::string kQRext = "_qrcode.png";

std::filesystem::path symbro::getQRPath(std::filesystem::path lnk)
{
    return mQR/(lnk.filename().string()+kQRext);
}
std::string symbro::getQRURL(std::filesystem::path lnk)
{
    return mURL+"/qr/"+lnk.filename().string()+kQRext;
}
