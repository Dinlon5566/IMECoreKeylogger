#include "pch.h"
#include "util.h"
#include "logger.h"
#include <windows.h>

// Static instance of Logger
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// Constructor
Logger::Logger() {}

// Destructor
Logger::~Logger() {
    closeAll();
}

// Initialize Logger
bool Logger::initialize(const wchar_t* dllPath) {
    std::wstring path(dllPath);
    size_t pos = path.find_last_of(L"\\/");
    if (pos == std::wstring::npos) {
        return false;
    }
    directory = path.substr(0, pos + 1);
    return true;
}

// Set log file
bool Logger::setLogFile(const std::wstring& logName) {
    std::lock_guard<std::mutex> lock(mtx);

    if (logFiles.find(logName) == logFiles.end()) {
        std::wstring fullPath = directory + logName;
        std::wofstream* ofs = new std::wofstream(fullPath, std::ios::app);
        if (!ofs->is_open()) {
            delete ofs;
            return false;
        }
        logFiles[logName] = ofs;
    }
    return true;
}

// Log message
void Logger::log(const std::wstring& logName, const std::wstring& message) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = logFiles.find(logName);
    if (it != logFiles.end()) {
        *(it->second) << message;
        it->second->flush();
    }
}

// Close all log files
void Logger::closeAll() {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& pair : logFiles) {
        if (pair.second->is_open()) {
            pair.second->close();
        }
        delete pair.second;
    }
    logFiles.clear();
}

// Initialize logger globally
void initializeLogger() {
    wchar_t dllPath[MAX_PATH];
    if (GetModuleFileNameW(NULL, dllPath, MAX_PATH)) {
        if (!Logger::getInstance().initialize(dllPath)) {
            // Handle error
        }
    }
}

void IMElogger(const std::wstring& message) {
    Logger::getInstance().setLogFile(L"IMElogger.txt");
    Logger::getInstance().log(L"IMElogger.txt", message);
}

void DEBUGlogger(const std::wstring& message) {
    Logger::getInstance().setLogFile(L"DEBUGlogger.txt");
    Logger::getInstance().log(L"DEBUGlogger.txt", message);
}

void IMEKeyInputlogger(const std::wstring& message) {
	Logger::getInstance().setLogFile(L"IMEKeylogger.txt");
	Logger::getInstance().log(L"IMEKeylogger.txt", message);
}