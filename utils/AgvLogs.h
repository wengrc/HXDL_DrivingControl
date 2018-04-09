#ifndef AGVLOGS_H
#define AGVLOGS_H

#include <iostream>
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/ndc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;
using namespace std;

class CAgvLogs
{
    public:
        static CAgvLogs &Instance();
        ~CAgvLogs();

    public:
        void Initial(const char *fname, const char *tag, int maxSize);
        void SaveInfo(string info);
        void SaveError(string error);
        void SaveDebug(string debug);

        static string ByteArrayToString(unsigned char *data, int len);
    private:
        CAgvLogs();
    public:
        Logger logger;
    private:
        static CAgvLogs *instance;
};

#define LogInfo(TAG, fmt, ...)      LOG4CPLUS_INFO_FMT(CAgvLogs::Instance().logger,   "[" TAG "] " fmt, ##__VA_ARGS__)
#define LogDebug(TAG, fmt, ...)     LOG4CPLUS_DEBUG_FMT(CAgvLogs::Instance().logger,  "[" TAG "] " fmt, ##__VA_ARGS__)
#define LogWarn(TAG, fmt, ...)      LOG4CPLUS_WARN_FMT(CAgvLogs::Instance().logger,   "[" TAG "] " fmt, ##__VA_ARGS__)
#define LogError(TAG, fmt, ...)     LOG4CPLUS_ERROR_FMT(CAgvLogs::Instance().logger,  "[" TAG "] " fmt, ##__VA_ARGS__)
#define LogFatal(TAG, fmt, ...)     LOG4CPLUS_FATAL_FMT(CAgvLogs::Instance().logger,  "[" TAG "] " fmt, ##__VA_ARGS__)
#define LogTrace(TAG, fmt, ...)     LOG4CPLUS_TRACE_FMT(CAgvLogs::Instance().logger,  "[" TAG "] " fmt, ##__VA_ARGS__)

#endif // AGVLOGS_H
