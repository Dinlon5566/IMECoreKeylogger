#include "pch.h"
#include "util.h"
#include "logger.h"
#include <windows.h>
#include <codecvt>
#include <locale>

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
bool Logger::setLogFile(const std::wstring& logName) {
    std::lock_guard<std::mutex> lock(mtx);

    if (logFiles.find(logName) == logFiles.end()) {
        std::wstring fullPath = directory + logName;

        {
            std::ifstream checkFile(fullPath, std::ios::binary);
            if (!checkFile.good()) {
                std::wofstream createFile(fullPath, std::ios::binary | std::ios::out);
                if (createFile.is_open()) {
                    createFile.imbue(std::locale(std::locale::empty(),
                        new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>()));
                    //  BOM
                    wchar_t bom = 0xFEFF;
                    createFile.write(&bom, 1);
                }
            }
        }

        std::wofstream* ofs = new std::wofstream(fullPath,
            std::ios::binary | std::ios::app);
        if (!ofs->is_open()) {
            delete ofs;
            return false;
        }

        ofs->imbue(std::locale(std::locale::empty(),
            new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>()));

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
    if (getDLLPath(dllPath)) {
        if (!Logger::getInstance().initialize(dllPath)) {
            if (DEBUG)
                MessageBoxW(NULL, L"Fail to initialize Logger", L"initializeLogger", MB_OK);
        }
    }
    else {
        if (DEBUG)
            MessageBoxW(NULL, L"Fail to get DLL path", L"initializeLogger", MB_OK);
    }
}

void DEBUGlogger(const std::wstring& message) {
    Logger::getInstance().setLogFile(L"DEBUGlogger.txt");
    Logger::getInstance().log(L"DEBUGlogger.txt", message);
}

void IMEKeyInputlogger(const std::wstring& message) {
	Logger::getInstance().setLogFile(L"IMEKeyInputlogger.txt");
	Logger::getInstance().log(L"IMEKeyInputlogger.txt", message);
}
