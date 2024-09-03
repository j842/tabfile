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

void symbro::erase()
{
    spdlog::info("- Deleting symlinks in {}",mLinks.string());
    deleteDirectoryContents(mLinks);
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
            {
                std::filesystem::path lnk = mLinks/(fname+p.extension().string());
                checklink(p,lnk);
                checkqr(lnk);
            }
        }

    std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now();
    spdlog::info("- Rescan completed in {} ms",std::chrono::duration_cast<std::chrono::milliseconds>(tend - tbegin).count());
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

            for (auto & w : writers)
                w->addrow(i,url,parent,origfile,qrcodelink);
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

        rescan();
        make_index();
    }
}

// returns true if changed.
bool symbro::checklink(std::filesystem::path linktarget, std::filesystem::path lnk)
{
    if (std::filesystem::is_symlink(lnk))
    { // is it up to date?
        std::filesystem::path orig = std::filesystem::read_symlink(lnk);
        if (std::filesystem::equivalent(orig,linktarget))
            return false;
    }

    if (std::filesystem::exists(lnk))
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

indexwriter_xlsx::indexwriter_xlsx(std::filesystem::path p)
{
    workbook = workbook_new(p.string().c_str());
    worksheet = workbook_add_worksheet(workbook, "Directory Links");

    worksheet_set_column(worksheet,0,1,50,nullptr);
    worksheet_set_column(worksheet,2,2,80,nullptr);
    worksheet_set_column(worksheet,3,3,60,nullptr);

    lxw_format *bold = workbook_add_format(workbook);
    format_set_bold(bold);
    format_set_bottom_color(bold,LXW_COLOR_BLACK );

    worksheet_write_string(worksheet, 0, 0, "Link", bold);
    worksheet_write_string(worksheet, 0, 1, "Parent", bold);
    worksheet_write_string(worksheet, 0, 2, "Original File", bold);
    worksheet_write_string(worksheet, 0, 3, "QR Code Link", bold);
}

indexwriter_xlsx::~indexwriter_xlsx()
{
    workbook_close(workbook);
}

void indexwriter_xlsx::addrow(int rowindex, const std::string &url, const std::string &parent, const std::string &origfile, const std::string &qrcodelink)
{
    worksheet_write_string(worksheet, rowindex, 0, url.c_str(), nullptr);
    worksheet_write_string(worksheet, rowindex, 1, parent.c_str(), nullptr);
    worksheet_write_string(worksheet, rowindex, 2, origfile.c_str(), nullptr);
    worksheet_write_string(worksheet, rowindex, 3, qrcodelink.c_str(), nullptr);
} 

indexwriter_html::indexwriter_html(std::filesystem::path p)
{
    ofs.open(p.string());

    ofs <<
R"HTMLFILE(
    <html>
    <head>
        <title>Directory Index</title>
        <style>
            table, th, td {
            border: 1px solid black;
            border-collapse: collapse;
            }
            th, td {
            padding: 15px;
            }
        </style>
    </head>
    <body>
        <table>
            <caption>
                Directory Index
            </caption>
        <thead>
            <tr>
            <th scope="col">Link</th>
            <th scope="col">Parent</th>
            <th scope="col">Original File</th>
            <th scope="col">QR Code</th>
            </tr>
        </thead>
        <tbody>
)HTMLFILE"; 
}

indexwriter_html::~indexwriter_html()
{
    ofs <<
R"HTMLFILE(
        </tbody>
    </body>
)HTMLFILE"; 

    ofs.close();
}

void indexwriter_html::addrow(int /*rowindex*/, const std::string &url, const std::string &parent, const std::string &origfile, const std::string &qrcodelink)
{
    ofs << "<tr>" << std::endl <<
    "<td><a href=\""<<url<<"\">"<<url<<"</a></td>"<<std::endl <<
    "<td>"<<parent<<"</td>"<<std::endl <<
    "<td>"<<origfile<<"</td>"<<std::endl <<
    "<td><a href=\""<<qrcodelink<<"\">"<<qrcodelink<<"</a></td>" <<std::endl <<
    "</tr>"<<std::endl;
}
