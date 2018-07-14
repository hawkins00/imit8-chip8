/*
 * logWriter
 * "It writes logs."
 */

#include "logWriter.h"

logWriter::logWriter()
{
    std::cout << "logWriter was instantiated without a file name.  Using \"log.txt\"" << std::endl;
    logWriter("log.txt");
}

logWriter::logWriter(std::string fileToOpen)
{
    std::cout << "Using \"" << fileToOpen << "\" as the log file." << std::endl;
    setOutputFileName("./" + fileToOpen);
    openFile(outputFileName);
}

logWriter::~logWriter()
{
    closeFile();
}

std::string& logWriter::getOutputFileName()
{
    return outputFileName;
}

void logWriter::setOutputFileName(const std::string& outputFileName)
{
    logWriter::outputFileName = outputFileName;
}

bool logWriter::openFile(std::string fileToOpen)
{
    outputStream.open(fileToOpen, std::ios_base::app);
    return outputStream.good();
}

bool logWriter::closeFile()
{
    outputStream.close();
    return true;
}

bool logWriter::writeToFile(std::string stringToWriteToLogfile)
{
    if (outputStream.is_open() && outputStream.good())
    {
        outputStream.write(stringToWriteToLogfile.c_str(), stringToWriteToLogfile.length());
        outputStream.write("\n", 1);
        return true;
    }

    std::cout << "ERROR:  Writing to file failed, outputStream is not open for writing." << std::endl;
    return false;
}

bool logWriter::log(std::string stringToWrite)
{
    return writeToFile(stringToWrite);
}

