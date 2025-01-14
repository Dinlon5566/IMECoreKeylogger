#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <map>
#include <fstream>
#include <mutex>

class Logger {
public:
    static Logger& getInstance();

    bool initialize(const wchar_t* dllPath);

    bool setLogFile(const std::wstring& logName);

    void log(const std::wstring& logName, const std::wstring& message);

    void closeAll();

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::wstring directory; // File directory
    std::map<std::wstring, std::wofstream*> logFiles;
    std::mutex mtx;
};

void initializeLogger();

void IMElogger(const std::wstring& message);
void DEBUGlogger(const std::wstring& message);
void IMEKeyInputlogger(const std::wstring& message);
void IMElpmsglogger(const std::wstring& message);

#endif // LOGGER_H
