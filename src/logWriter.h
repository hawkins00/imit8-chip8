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
        logWriter::~logWriter();

        struct logLevel
        {
            enum level
            {
                ERROR, WARNING, INFO, DEBUG,
            };

            inline static std::string to_string(level l)
            {
                switch (l)
                {
                    case ERROR:
                        return "ERROR";
                    case WARNING:
                        return "WARNING";
                    case INFO:
                        return "INFO";
                    case DEBUG:
                        return "DEBUG";
                    default:
                        return "UNKNOWN";
                }
            }
        };

        std::string& logWriter::getOutputFileName();
        bool log(logLevel::level level, std::string stringToWrite);

    private:
        std::string outputFileName = "";
        std::ofstream outputStream;

        void setOutputFileName(const std::string& outputFileName);
        bool openFile(std::string fileToOpen);
        bool closeFile();
        bool writeToFile(logLevel::level level, std::string stringToWriteToLogfile);
};

#endif //IMIT8_CHIP8_LOGWRITER_H
