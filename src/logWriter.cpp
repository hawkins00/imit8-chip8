/*
 * logWriter
 * "It writes logs."
 */

#include "logWriter.h"

logWriter::
logWriter()
{
    std::cout
            << "logWriter was instantiated without a file name.  Using \"log.txt\" and ERROR as the default logging level."
            << std::endl;
    logWriter("log.txt", logLevel::level::ERROR);
}

logWriter::
logWriter(std::string fileToOpen, logLevel::level level)
{
    std::cout << "Using \"" << fileToOpen << "\" as the log file." << std::endl;
    setOutputFileName("./" + fileToOpen);
    setCurrentLoggingLevel(level);
    openFile(outputFileName);
}

logWriter::
~logWriter()
{
    closeFile();
}

std::string& logWriter::
getOutputFileName()
{
    return outputFileName;
}

void logWriter::
setOutputFileName(const std::string& outputFileName)
{
    logWriter::outputFileName = outputFileName;
}

bool logWriter::
openFile(std::string fileToOpen)
{
    outputStream.open(fileToOpen, std::ios_base::app);
    return outputStream.good();
}

bool logWriter::
closeFile()
{
    outputStream.close();
    return true;
}

bool logWriter::
writeToFile(std::string levelOfMessage, std::string& stringToWriteToLogfile)
{
    if (outputStream.is_open() && outputStream.good())
    {
        // TODO:  Print timestamp on logs.
        outputStream.write("[", 1);
        outputStream.write(levelOfMessage.c_str(), levelOfMessage.length());
        outputStream.write("]   ", 4);

        outputStream.write(stringToWriteToLogfile.c_str(), stringToWriteToLogfile.length());
        outputStream.write("\n", 1);
        return true;
    }

    std::cout << "ERROR:  Writing to file failed, outputStream is not open for writing." << std::endl;
    return false;
}

bool logWriter::
log(logLevel::level levelOfMessage, std::string stringToWrite)
{
    if (levelOfMessage <= getCurrentLoggingLevel())
    {
        return writeToFile(logLevel::to_string(levelOfMessage), stringToWrite);
    }
    return false;
}

logWriter::logLevel::level logWriter::
getCurrentLoggingLevel() const
{
    return currentLoggingLevel;
}

void logWriter::
setCurrentLoggingLevel(logWriter::logLevel::level currentLoggingLevel)
{
    logWriter::currentLoggingLevel = currentLoggingLevel;
}
