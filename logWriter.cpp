/*
 * logWriter
 * "It writes logs."
 */

#include "logWriter.h"

logWriter::logWriter() {
    std::cout << "logWriter was instantiated without a file name.  Using \"log.txt\"" << std::endl;
    setOutputFileName("log.txt");
}

logWriter::logWriter(std::string fileToOpen) {
    std::cout << "Using \"" << fileToOpen << "\" as the log file." << std::endl;
    setOutputFileName(fileToOpen);
}

const std::string &logWriter::getOutputFileName() const {
    return outputFileName;
}

void logWriter::setOutputFileName(const std::string &outputFileName) {
    logWriter::outputFileName = outputFileName;
}

bool logWriter::openFile(std::string fileToOpen) {
    myFile.open(fileToOpen, std::ios_base::app);
    return true;
}

bool logWriter::closeFile() {
    myFile.close();
    return true;
}

bool logWriter::writeToFile(std::string stringToWriteToLogfile) {
    if(myFile.is_open() && myFile.good()) {
        myFile.write(stringToWriteToLogfile.c_str(), stringToWriteToLogfile.length());
        return true;
    }
    return false;
}

bool logWriter::log(std::string stringToWrite) {
    writeToFile(stringToWrite);
    return false;
}

