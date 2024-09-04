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

    worksheet_write_string(worksheet, 0, 0, "Link", bold);
    worksheet_write_string(worksheet, 0, 1, "Parent", bold);
    worksheet_write_string(worksheet, 0, 2, "Original File", bold);
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
        <style>
            table, th, td {
                color: #242424;
                border: 1px solid black;
                border-collapse: collapse;
            }
            th, td {
                padding: 10px;
            }
            tr:nth-child(even) {
                background-color: #dddddd;
            }             
            tr:nth-child(odd) {
                background-color: #fefefe;
            }
            body {
                color: #fefefe;
                background-color: #242424;
            }
        </style>

        <script>
            function copyToClipboard(copyText) {
            navigator.clipboard.writeText(copyText);
            alert("Copied the text: " + copyText);
            }
        </script>
    </head>
    <body>
        <center>
            <p>Directory Index</p>
        <table>
        <thead>
            <tr>
            <th scope="col">Link</th>
            <th scope="col">Parent</th>
            <th scope="col">Original File</th>
            <th scope="col">QR Code</th>
            <th scope="col">Markdown</th>
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
    "<td><a href=\""<<url<<"\">"<<url<<"</a></td>"<<std::endl <<
    "<td>"<<parent<<"</td>"<<std::endl <<
    "<td>"<<origfile<<"</td>"<<std::endl <<
    "<td><a href=\""<<qrcodelink<<"\">QR Code</a></td>" <<std::endl <<
    "<td><button onclick=\"copyToClipboard('"+markdown.str()+"')\">Markdown</button></td>" <<std::endl <<
    "</tr>"<<std::endl;
}
