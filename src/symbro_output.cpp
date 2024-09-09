#include <spdlog/spdlog.h>

#include "symbro.h"
#include "utils.h"



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

    worksheet_write_string(worksheet, 0, 0, "PermaLink", bold);
    worksheet_write_string(worksheet, 0, 1, "Location", bold);
    worksheet_write_string(worksheet, 0, 2, "Target File", bold);
    worksheet_write_string(worksheet, 0, 3, "QR Code Link", bold);
}

indexwriter_xlsx::~indexwriter_xlsx()
{
    workbook_close(workbook);
}

void indexwriter_xlsx::addrow(int rowindex, const std::string &url, 
                            const std::string &parent, 
                            const std::string &origfile, 
                            const std::string &qrcodelink,
                            const std::string & /*title*/)
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
        
        <script src="//ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>
        <link rel="stylesheet" type="text/css" href="//cdn.datatables.net/2.1.5/css/dataTables.dataTables.min.css">
        <script src="//cdn.datatables.net/2.1.5/js/dataTables.min.js"></script>
        <script src="//cdnjs.cloudflare.com/ajax/libs/jquery-toast-plugin/1.3.2/jquery.toast.min.js"
                integrity="sha512-zlWWyZq71UMApAjih4WkaRpikgY9Bz1oXIW5G0fED4vk14JjGlQ1UmkGM392jEULP8jbNMiwLWdM8Z87Hu88Fw=="
                crossorigin="anonymous" referrerpolicy="no-referrer"></script>
        <link rel="stylesheet" href="//cdnjs.cloudflare.com/ajax/libs/jquery-toast-plugin/1.3.2/jquery.toast.min.css"
            integrity="sha512-wJgJNTBBkLit7ymC6vvzM1EcSWeM9mmOu+1USHaRBbHkm6W9EgM0HY27+UtUaprntaYQJF75rc8gjxllKs5OIQ=="
            crossorigin="anonymous" referrerpolicy="no-referrer" />

    <script>
        function copyToClipboard(copyText) {
            navigator.clipboard.writeText(copyText);
            console.log("copyText", copyText)
            $.toast({
                text: "Copied the text:<br>" + copyText,
                hideAfter: 3010
            });
            }

        $(document).ready(function() {
            $("#linksTable").DataTable({
                paging: false
            });


        });

    </script>
</head>
    <body>
        <center>
            <p>Directory Index</p>
        <table id="linksTable">
        <thead>
            <tr>
            <th scope="col">Location</th>
            <th scope="col">PermaLink</th>
            <th scope="col">QR Code</th>
            <th scope="col">Markdown</th>
            <th scope="col">Current Target File</th>
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
        </center>
    </body>
)HTMLFILE"; 

    ofs.close();
}

void indexwriter_html::addrow(int /*rowindex*/, const std::string &url, 
                            const std::string &parent, 
                            const std::string &origfile, 
                            const std::string &qrcodelink,
                            const std::string & title)
{
    std::ostringstream markdown;
    markdown << title << ": [PermaLink]";
    markdown << "(" << url << "){target=_blank} ";
    markdown << "([QR Code]("<<qrcodelink<<"){target=_blank})";

    ofs << "<tr>" << std::endl <<

    "<td>"<<parent<<"</td>"<<std::endl <<
    "<td><a href=\""<<url<<"\">PermaLink</a></td>"<<std::endl <<
    "<td><a href=\""<<qrcodelink<<"\">QR Code</a></td>" <<std::endl <<
    "<td><button onclick=\"copyToClipboard('"+markdown.str()+"')\">Markdown</button></td>" <<std::endl <<
    "<td>"<<origfile<<"</td>"<<std::endl <<

    "</tr>"<<std::endl;
}
