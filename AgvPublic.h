#ifndef AGVPUBLIC_H
#define AGVPUBLIC_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include <math.h>

#include <map>
#include <vector>
#include <list>


#include "utils/AgvUtils.h"
#include "utils/AgvCheckSum.h"
#include "utils/AgvXml.h"
#include "utils/AgvLogs.h"
#include "utils/AgvThread.h"
#include "utils/AgvMathUtils.h"

#include "event/AgvEvent.h"
#include "event/AgvEventHelper.h"

#include "main/AgvSetting.h"

#include "devices/canbus/AgvSocketCan.h"

//#define ZZS
//#define HX
#define CryControler

/*************导航方式********************/
#define     LidarGuideForklift  4
/**************AGV相关参数****************/
#define     CarWheelSperation        0
/***************************************/
//#define     CarWheelBase        2130//2080
//#define     CarWheelRadius      115
//#undef      GearRedutionRatio
//#define     GearRedutionRatio   23.3

//hx
//#define     CarWheelBase        1150//1150//1300
//#define     CarWheelRadius      130
//#undef      GearRedutionRatio
//#define     GearRedutionRatio   14


#define     GuidanceMode        LidarGuideForklift
#define     EnableLidar
#define     EnableHMI


#endif // AGVPUBLIC_H


