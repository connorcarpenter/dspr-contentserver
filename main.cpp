#include <uWS/uWS.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

struct ContentFile
{
    std::stringstream* ss;
    long modified;
};

std::map<std::string, ContentFile> fileMap;

std::stringstream* getFileStream(std::string fileName);

bool loadFile(std::string fileName);

int main() {
    uWS::Hub h;
    uint portToRun = 3170;

    h.onHttpRequest([&](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t length, size_t remainingBytes)
    {
            if (strncmp(req.headers->value, "/orchestrator/", 14) == 0)
            {
                //call orchestrator service
                res->end("ws://www.deuspora.com:3172", 26);

                std::string path = std::string(req.headers->value, req.headers->valueLength);
                std::cout << "dspr-contentserver: Called through to orchestrator service: " << path << std::endl;
                return;
            }
            else
            {
                std::string fileName = std::string(req.headers->value, req.headers->valueLength);
                std::stringstream* file = getFileStream(fileName);

                if (file == nullptr) {
                    res->end(nullptr, 0);
                    return;
                }

                res->end(file->str().data(), file->str().length());

                std::cout << "dspr-contentserver: Served file: " << fileName << std::endl;
                return;
            }
    });

    h.getDefaultGroup<uWS::SERVER>().startAutoPing(30000);
    if (h.listen("localhost", portToRun)) {
        std::cout << "dspr-contentserver: Listening to port " << portToRun << "" << std::endl;
    } else {
        std::cerr << "dspr-contentserver: Failed to listen to port " << portToRun << "" << std::endl;
        return -1;
    }

    h.run();
}

std::string filePathBase = "/home/connor/Work/fips-deploy/dspr-frontend/wasm-make-release";
struct stat result;

std::stringstream* getFileStream(std::string fileName)
{
    //check whether item exists
    auto it = fileMap.find(fileName);
    if(it != fileMap.end())
    {
        //item exists already in map

        //let's find last modified time for content
        std::string completeFilePath = filePathBase + fileName;

        if(stat(completeFilePath.c_str(), &result) != 0) {
            std::cerr << "dspr-contentserver: could not get modified time for: " << fileName << std::endl;
            return nullptr;
        }

        long newTimeModified = result.st_mtime;

        auto contentFile = fileMap.at(fileName);

        if (newTimeModified == contentFile.modified)
            return contentFile.ss;

        std::cout << "dspr-contentserver: Reloading file into memory: " << fileName << std::endl;
        fileMap.erase(fileName);
        if(loadFile(fileName))
            return fileMap.at(fileName).ss;
        return nullptr;
    }
    else
    {
        //item does not exist in map
        std::cout << "dspr-contentserver: Loading file into memory: " << fileName << std::endl;
        if(loadFile(fileName))
            return fileMap.at(fileName).ss;
        return nullptr;
    }
}

bool loadFile(std::string fileName)
{
    std::string completeFilePath = filePathBase + fileName;

    if(stat(completeFilePath.c_str(), &result) != 0) {
        std::cerr << "dspr-contentserver: could not get modified time for: " << fileName << std::endl;
        return false;
    }

    auto mod_time = result.st_mtime;


    std::stringstream* newFile = new std::stringstream();
    (*newFile) << std::ifstream (completeFilePath).rdbuf();
    if (!newFile->str().length()) {
        std::cerr << "dspr-contentserver: Failed to load: " << fileName << std::endl;
        return false;
    }

    ContentFile contentFile;
    contentFile.ss = newFile;
    contentFile.modified = mod_time;

    fileMap.insert(std::pair<std::string, ContentFile>(fileName, contentFile));

    return true;
}
