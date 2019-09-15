/**
 * Copyright (c) Chris Kim & Matt Hawkins
 * This program is licensed under the "GPLv3 License"
 * Please see the file License.md in the source
 * distribution of this software for license terms.
 */

 /*
 * LogWriter
 * "It writes logs."
 */

#ifndef IMIT8_CHIP8_LOGWRITER_H
#define IMIT8_CHIP8_LOGWRITER_H

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>

class LogWriter
{
    public:
        struct LogLevel
        {
            enum Level
            {
                ERROR, WARNING, INFO, DEBUG,
            };

            inline static std::string to_string(Level l)
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

        explicit LogWriter(std::string fileToOpen = "log.txt", LogLevel::Level level = LogLevel::Level::INFO);
        ~LogWriter();

        std::string& getOutputFileName();
        bool log(LogLevel::Level levelOfMessage, std::string stringToWrite);

    private:
        std::string outputFileName = "";
        std::ofstream outputStream;
        LogLevel::Level currentLoggingLevel;
        bool isFreshLog;

        void setOutputFileName(const std::string& outputFileName);
        bool openFile(std::string fileToOpen);
        bool closeFile();
        bool writeToFile(std::string levelOfMessage, std::string& stringToWriteToLogfile);

        LogLevel::Level getCurrentLoggingLevel() const;
        void setCurrentLoggingLevel(LogLevel::Level currentLoggingLevel);
};

#endif //IMIT8_CHIP8_LOGWRITER_H
