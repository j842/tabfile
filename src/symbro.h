#ifndef __SYMBRO_H
#define __SYMBRO_H

#include <string>
#include <filesystem>

class symbro
{
    public:
        symbro(std::filesystem::path p, std::string baseURL);

        void erase();
        void rescan();
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
        std::filesystem::path _mOutput;

        std::filesystem::path mLinks;
        std::filesystem::path mQR;

        std::string mURL;
};


#endif