#ifndef __SYMBRO_H
#define __SYMBRO_H

#include <string>
#include <fstream>
#include <filesystem>
#include <xlsxwriter.h>

class indexwriter
{
    public:
        virtual ~indexwriter() {};

        virtual void addrow(int rowindex,
                            const std::string & url,
                            const std::string & parent,
                            const std::string & origfile,
                            const std::string & qrcodelink,
                            const std::string & title) =0;
};

class indexwriter_xlsx : public indexwriter
{
    public:
        indexwriter_xlsx(std::filesystem::path p);
        ~indexwriter_xlsx();

        void addrow(int rowindex,
                        const std::string & url,
                        const std::string & parent,
                        const std::string & origfile,
                        const std::string & qrcodelink,
                        const std::string & title);

    private:
        lxw_workbook  * workbook;
        lxw_worksheet * worksheet;

};

class indexwriter_html : public indexwriter
{
    public:
        indexwriter_html(std::filesystem::path p);
        ~indexwriter_html();

        void addrow(int rowindex,
                        const std::string & url,
                        const std::string & parent,
                        const std::string & origfile,
                        const std::string & qrcodelink,
                        const std::string & title);

    private:
        std::ofstream ofs;
};


class symbro
{
    public:
        symbro(std::filesystem::path p, std::string baseURL);

        //void erase();
        int rescan();
        void make_index();

        void watch(); // blocks forever!

    private:
        bool checklink(std::filesystem::path linktarget, std::filesystem::path lnk);
        bool checkqr(std::filesystem::path lnk);

        std::string getURL(std::filesystem::path lnk);
        std::filesystem::path getQRPath(std::filesystem::path lnk);
        std::string getQRURL(std::filesystem::path lnk);

    private:
        std::filesystem::path mDir;
        std::filesystem::path mSource;
        std::filesystem::path mOutput;

        std::filesystem::path mLinks;
        std::filesystem::path mQR;

        std::string mURL;
};


#endif