/*
 * logWriter
 * "It writes logs."
 */

#ifndef IMIT8_CHIP8_LOGWRITER_H
#define IMIT8_CHIP8_LOGWRITER_H

#include <string>
#include <iostream>
#include <fstream>

class logWriter
{

    public:
        logWriter();

        logWriter(std::string fileToOpen);

        const std::string &getOutputFileName() const;

        bool log(std::string stringToWrite);

    private:
        std::string outputFileName = "";
        std::ofstream myFile;

        void setOutputFileName(const std::string &outputFileName);

        bool openFile(std::string fileToOpen);

        bool closeFile();

        bool writeToFile(std::string stringToWriteToLogfile);


};


#endif //IMIT8_CHIP8_LOGWRITER_H
