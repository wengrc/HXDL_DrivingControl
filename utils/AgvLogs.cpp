#include "AgvLogs.h"

CAgvLogs *CAgvLogs::instance = NULL;

CAgvLogs::CAgvLogs()
{

}

CAgvLogs::~CAgvLogs()
{

}

CAgvLogs &CAgvLogs::Instance()
{
    if (instance == NULL)
    {
        instance = new CAgvLogs();
    }

    return *instance;
}

void CAgvLogs::Initial(const char *fname, const char *tag, int maxsize)
{
    log4cplus::initialize ();
    //helpers::LogLog::getLogLog()->setInternalDebugging(true);
    SharedFileAppenderPtr append(new RollingFileAppender(LOG4CPLUS_TEXT(fname), maxsize * 1024 * 1024, 1, false, true));
    append->setName(LOG4CPLUS_TEXT(tag));

    string format = "[%p] %d{%Y-%m-%d %H:%M:%S.%q} - %m%n";
    append->setLayout( std::auto_ptr<Layout>(new PatternLayout(format)) );
    append->getloc();

    SharedAppenderPtr ttyappend(new ConsoleAppender());
    ttyappend->setName(LOG4CPLUS_TEXT(tag));
    ttyappend->setLayout( std::auto_ptr<Layout>(new PatternLayout(format)) );

    logger = Logger::getInstance(LOG4CPLUS_TEXT("logger"));
    logger.addAppender(SharedAppenderPtr(append.get()));
    logger.addAppender(SharedAppenderPtr(ttyappend.get()));

}

void CAgvLogs::SaveInfo(string info)
{
    LOG4CPLUS_INFO(logger, info);
}

void CAgvLogs::SaveDebug(string debug)
{
    LOG4CPLUS_DEBUG(logger, debug);
}

void CAgvLogs::SaveError(string error)
{
    LOG4CPLUS_ERROR(logger, error);
}

string CAgvLogs::ByteArrayToString(unsigned char *data, int len)
{
    const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7','8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    string str(len * 3-1, ' ');

    for (int i = 0; i < len; ++i)
    {
        str[3 * i]     = hexmap[(data[i] & 0xF0) >> 4] ;
        str[3 * i + 1] = hexmap[data[i] & 0x0F];

        if ( (3*i+2) != (3*len-1) )
        {
            str[3 * i + 2] = ' ';
        }

    }

    return str;
}


