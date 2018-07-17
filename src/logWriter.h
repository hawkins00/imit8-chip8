/*
 * logWriter
 * "It writes logs."
 */

#ifndef IMIT8_CHIP8_LOGWRITER_H
#define IMIT8_CHIP8_LOGWRITER_H

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>

class logWriter
{
    public:
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

        logWriter();
        logWriter(std::string fileToOpen, logLevel::level level);
        ~logWriter();

        std::string& getOutputFileName();
        bool log(logLevel::level levelOfMessage, std::string stringToWrite);

    private:
        std::string outputFileName = "";
        std::ofstream outputStream;
        logLevel::level currentLoggingLevel;

        void setOutputFileName(const std::string& outputFileName);
        bool openFile(std::string fileToOpen);
        bool closeFile();
        bool writeToFile(std::string levelOfMessage, std::string& stringToWriteToLogfile);

        logLevel::level getCurrentLoggingLevel() const;
        void setCurrentLoggingLevel(logLevel::level currentLoggingLevel);
};

#endif //IMIT8_CHIP8_LOGWRITER_H
