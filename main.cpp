#include "main/AgvMainLoop.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include <time.h>
#include <execinfo.h>

#include "AgvPublic.h"
#include "controler/AgvCar.h"
#include "devices/network/AgvWifi.h"
#include "devices/network/AgvNetManager.h"
#include "comm/AgvFinder.h"

using namespace std;

#define LOGTAG          "Main"
#define LOGDIR          "LOGS/"
#define CRASHLOGFILE    "crashlog.txt"
#define XMLFILENAME     "config.xml"


const char *VERSION = "3.7.0.0.2";
#define BUILDTIME   __DATE__ " " __TIME__

CAgvMainLoop mainLoop;
CAgvCar      agvCar;
CAgvSetting * gAgvSetting = NULL;
CAgvFinder   agvReplier(CAgvFinder::Replier);


char gCurrentIp[30] = {"192.168.1.222"};


void SignalHandler(int sig)
{
    DEBUG_INFO("Main", "Get Signal:%d", sig);
    if(sig == SIGSEGV || sig == SIGABRT || sig == SIGILL)
    {
        FILE *logf;
        char  log[10240];
        time_t  cur;
        struct tm *p;

        time(&cur);
        p = gmtime(&cur);

        if (system("touch "LOGDIR CRASHLOGFILE )) {;}
        sprintf(log,"\nAppllication crashed at %04d/%02d/%02d %02d:%02d:%02d !!!!\n",
                                            p->tm_year+1900,
                                            p->tm_mon + 1,
                                            p->tm_mday,
                                            p->tm_hour,
                                            p->tm_min,
                                            p->tm_sec);
        logf = fopen(LOGDIR CRASHLOGFILE,"a+");
        if (logf == NULL)
        {
           perror("Open crashlog file failed!!");
        }


        void   *array[125];
        size_t size;
        char   **strings;
        size_t i;

        size    = backtrace(array,125);
        strings = backtrace_symbols(array,size);
        sprintf(log + strlen(log),"=======Obtained %zd stack frames=======\n",size);
        for (i = 0; i < size; i++)
        {
            sprintf(log + strlen(log),"%s\n",strings[i]);
        }
        free(strings);
        printf("%s",log);
        if (logf > 0)
        {
            fseek(logf,0,SEEK_END);
            int bufsize = 1024,total = strlen(log),
                blkcnt = total / bufsize,
                left = total % bufsize;
            int wret = -1;
            if (blkcnt)
            {
                wret = fwrite(log,bufsize,blkcnt,logf);
            }
            if (left)
            {
                wret = fwrite(log + bufsize * blkcnt,left,1,logf);
            }
            fflush(logf);
            fclose(logf);
        }
        sync();


        char buf[1024];
        char cmd[1024];
        FILE *fh;

        snprintf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());
        if(!(fh = fopen(buf, "r")))
        {
            exit(0);
        }
        if(!fgets(buf, sizeof(buf), fh))
        {
            exit(0);
        }
        fclose(fh);
        if(buf[strlen(buf) - 1] == '\n')
        {
            buf[strlen(buf) - 1] = '\0';
        }
        snprintf(cmd, sizeof(cmd), "gdb -ex 'thread apply all bt' %s %d", buf, getpid());
        if (system(cmd))
        {
//            printf("\n!!!!!!Now will reboot after 10 secs...\n");
//            sleep(10);
//            exit(1);
        }
        exit(1);
    }

    if (sig == SIGINT)
    {
        SendUrgentEvent(evSysClose, 0, NULL);
    }
}



int main()
{
    struct sigaction segact;
    segact.sa_handler = SignalHandler;
    sigemptyset(&segact.sa_mask);
    segact.sa_flags = 0;
    sigaction(SIGSEGV,  &segact,NULL);
    sigaction(SIGABRT,  &segact,NULL);
    sigaction(SIGILL,   &segact,NULL);
    sigaction(SIGINT,   &segact,NULL);


#if 0

    char *outBuffer;

    outBuffer = "rm /opt/time.txt";
    system(outBuffer);

    outBuffer = "rm /opt/error.txt";
    system(outBuffer);
#endif



    CAgvLogs::Instance().Initial("LOGS/agv.log", "MagNail", 3);
    LogInfo(LOGTAG, "Application Ver:%s BuildTime:%s started...", VERSION, BUILDTIME);

    gAgvSetting = &CAgvSetting::Instance();
    gAgvSetting->LoadAll(XMLFILENAME);


    CAgvNetManager::Instance().Init();//Get net para
    CAgvNetManager::Instance().SetupNetwork();


    agvReplier.SetAgvId(gAgvSetting->Id);
    agvReplier.SetAgvPid(gAgvSetting->Pid);
    agvReplier.SetLocalIp(gAgvSetting->LocalIp);
//  agvReplier.StartService(8200);


    agvCar.Init();
    mainLoop.StartLoop();
    mainLoop.ExecLoop();

    agvReplier.StopService();
    CAgvNetManager::Instance().CloseNetowrk(false);
    LogInfo(LOGTAG, "Mainloop exited");
    agvCar.Destroy();
    LogInfo(LOGTAG, "AgvCar destoryed");
    LogInfo(LOGTAG, "Application stoped");
    return 0;
}

