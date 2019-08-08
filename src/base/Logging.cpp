#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

#include "base/Logging.h"
#include "base/AsyncLogging.h"

Logger::LogLevel Logger::mLogLevel = Logger::LogDebug;
std::string Logger::mLogFile = "/dev/stdout";
bool Logger::mIsStdout = true;

Logger::Logger() :
    mCurPtr(mData)
{

}

Logger::~Logger()
{
    if(mIsStdout)
        printf("%s", mData);
    else
        AsyncLogging::instance()->append(mData, mCurPtr-mData);
}

void Logger::setLogFile(std::string file)
{
    Logger::mLogFile = file;
    if(Logger::mLogFile == "/dev/stdout")
        Logger::mIsStdout = true;
    else
        Logger::mIsStdout = false;
}

std::string Logger::getLogFile()
{
    return Logger::mLogFile;
}

void Logger::setLogLevel(LogLevel level)
{
    Logger::mLogLevel = level;
}

Logger::LogLevel Logger::getLogLevel()
{
    return Logger::mLogLevel;
}

void Logger::write(Logger::LogLevel level, const char* file, const char* func,
                            int line, const char* format, ...)
{
    if(level > Logger::mLogLevel)
        return;
    
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    struct tm* sysTime = localtime(&(now.tv_sec));

    mThisLogLevel = level;

    sprintf(mCurPtr, "%d-%02d-%02d %02d:%02d:%02d",
                sysTime->tm_year + 1900, sysTime->tm_mon + 1, sysTime->tm_mday,
                sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec);
    mCurPtr += strlen(mCurPtr);

    if(level == Logger::LogDebug)
    {
        sprintf(mCurPtr, " <DEBUG> ");
    }
    else if(level == Logger::LogWarning)
    {
        sprintf(mCurPtr, " <WARNING> ");
    }
    else if(level == Logger::LogError)
    {
        sprintf(mCurPtr, " <ERROR> ");
    }
    else
    {
        return;
    }
    mCurPtr += strlen(mCurPtr);

    sprintf(mCurPtr, "%s:%s:%d ", file, func, line);
    mCurPtr += strlen(mCurPtr);

    va_list valst;
    va_start(valst, format);

    vsnprintf(mCurPtr, sizeof(mData)-(mCurPtr-mData), format, valst);

    va_end(valst);

    mCurPtr += strlen(mCurPtr);
}