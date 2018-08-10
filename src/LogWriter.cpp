/**
 * Copyright (c) 2018 Chris Kim & Matt Hawkins
 * This program is licensed under the "GPLv3 License"
 * Please see the file License.md in the source
 * distribution of this software for license terms.
 */

 /*
 * logWriter
 * "It writes logs."
 */

#include "LogWriter.h"

LogWriter::
LogWriter()
{
    std::cout << "logWriter was instantiated without a file name.  Using \"log.txt\" and ERROR as the maximum logging level." << std::endl;
    LogWriter("log.txt", LogLevel::Level::ERROR);
}

LogWriter::
LogWriter(std::string fileToOpen, LogLevel::Level level)
{
    std::cout << "Using \"" << fileToOpen << "\" as the log file." << std::endl;
    setOutputFileName("./" + fileToOpen);
    setCurrentLoggingLevel(level);
    openFile(outputFileName);
}

LogWriter::
~LogWriter()
{
    closeFile();
}

std::string& LogWriter::
getOutputFileName()
{
    return outputFileName;
}

void LogWriter::
setOutputFileName(const std::string& outputFileName)
{
    LogWriter::outputFileName = outputFileName;
}

bool LogWriter::
openFile(std::string fileToOpen)
{
    outputStream.open(fileToOpen, std::ofstream::app);
    return outputStream.good();
}

bool LogWriter::
closeFile()
{
    outputStream.close();
    return true;
}

bool LogWriter::
writeToFile(std::string levelOfMessage, std::string& stringToWriteToLogfile)
{
    if (outputStream.is_open() && outputStream.good())
    {
        time_t now = time(NULL);
        std::string logTime = ctime(&now);
        outputStream.write(logTime.c_str(), logTime.length() - 1); // for some reason this adds a newline character, so -1
        outputStream.write("  [", 3);
        outputStream.write(levelOfMessage.c_str(), levelOfMessage.length());
        outputStream.write("]  ", 3);
        outputStream.write(stringToWriteToLogfile.c_str(), stringToWriteToLogfile.length());
        outputStream.write("\n", 1);
        outputStream.flush();
        return true;
    }

    std::cerr<< "ERROR:  Writing to file failed, outputStream is not open for writing." << std::endl;
    return false;
}

bool LogWriter::
log(LogLevel::Level levelOfMessage, std::string stringToWrite)
{
    if (levelOfMessage <= getCurrentLoggingLevel())
    {
        return writeToFile(LogLevel::to_string(levelOfMessage), stringToWrite);
    }
    return false;
}

LogWriter::LogLevel::Level LogWriter::
getCurrentLoggingLevel() const
{
    return currentLoggingLevel;
}

void LogWriter::
setCurrentLoggingLevel(LogWriter::LogLevel::Level currentLoggingLevel)
{
    LogWriter::currentLoggingLevel = currentLoggingLevel;
}
