TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += debug
TARGET = ForkLift
OBJECTS_DIR = temp
MOC_DIR = temp

SOURCES += \
    main.cpp \
    utils/AgvXmlSetting.cpp \
    utils/AgvXml.cpp \
    utils/AgvUtils.cpp \
    utils/AgvThread.cpp \
    utils/AgvLogs.cpp \
    utils/AgvCheckSum.cpp \
    event/AgvEventHelper.cpp \
    main/AgvSetting.cpp \
    main/AgvMainLoop.cpp \
    devices/network/AgvWifi.cpp \
    devices/network/AgvEthernet.cpp \
    devices/network/AgvNetManager.cpp \
    devices/canbus/AgvSensor.cpp \
    devices/canbus/AgvSensorManager.cpp \
    devices/canbus/AgvSocketCan.cpp \
    devices/canbus/AgvDownloadBin.cpp \
    devices/canbus/AgvCanOperator.cpp \
    controler/AgvScheduler.cpp \
    controler/AgvRoutePlanner.cpp \
    controler/AgvCar.cpp \
    controler/AgvCarControler.cpp \
    comm/AgvSocketServer.cpp \
    comm/AgvSocketClient.cpp \
    comm/AgvSchProtocol.cpp \
    comm/AgvJson.cpp \
    comm/AgvUdpSocket.cpp \
    comm/AgvFinder.cpp \
    utils/AgvMathUtils.cpp \
    controler/AgvMoveHelper.cpp \
    controler/AgvMoveHelperForklift.cpp \
    controler/AgvBaseControler.cpp \
    controler/AgvSteeringBaseControler.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    AgvPublic.h \
    utils/AgvXmlSetting.h \
    utils/AgvXml.h \
    utils/AgvUtils.h \
    utils/AgvThread.h \
    utils/AgvLogs.h \
    utils/AgvCheckSum.h \
    event/AgvEventHelper.h \
    event/AgvEvent.h \
    main/AgvSetting.h \
    main/AgvMainLoop.h \
    devices/network/AgvWifi.h \
    devices/network/AgvEthernet.h \
    devices/network/AgvNetManager.h \
    devices/canbus/AgvSensor.h \
    devices/canbus/AgvSensorManager.h \
    devices/canbus/AgvSocketCan.h \
    devices/canbus/AgvDownloadBin.h \
    devices/canbus/AgvCanOperator.h \
    controler/AgvScheduler.h \
    controler/AgvRoutePlanner.h \
    controler/AgvCar.h \
    controler/AgvCarControler.h \
    comm/AgvSocketServer.h \
    comm/AgvSocketClient.h \
    comm/AgvSchProtocol.h \
    comm/AgvJson.h \
    comm/AgvHttp.h \
    comm/AgvUdpSocket.h \
    comm/AgvFinder.h \
    utils/AgvMathUtils.h \
    controler/AgvMoveHelper.h \
    controler/AgvMoveHelperForklift.h \
    controler/AgvBaseControler.h \
    controler/AgvSteeringBaseControler.h


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../usr/local/arm/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/lib/release/ -lpthread
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../usr/local/arm/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/lib/debug/ -lpthread
else:unix: LIBS += -L$$PWD/../../../../../usr/local/arm/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/lib/ -lpthread

INCLUDEPATH += $$PWD/../../../../../usr/local/arm/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/include
DEPENDPATH += $$PWD/../../../../../usr/local/arm/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/arm-fsl-linux-gnueabi/multi-libs/usr/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/libcurl/lib/release/ -lcurl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/libcurl/lib/debug/ -lcurl
else:unix: LIBS += -L$$PWD/3rdparty/libcurl/lib/ -lcurl

INCLUDEPATH += $$PWD/3rdparty/libcurl/include
DEPENDPATH += $$PWD/3rdparty/libcurl/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/libjsonc/lib/release/ -ljson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/libjsonc/lib/debug/ -ljson
else:unix: LIBS += -L$$PWD/3rdparty/libjsonc/lib/ -ljson

INCLUDEPATH += $$PWD/3rdparty/libjsonc/include
DEPENDPATH += $$PWD/3rdparty/libjsonc/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/log4clib/lib/release/ -llog4cplus
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/log4clib/lib/debug/ -llog4cplus
else:unix: LIBS += -L$$PWD/3rdparty/log4clib/lib/ -llog4cplus

INCLUDEPATH += $$PWD/3rdparty/log4clib/include
DEPENDPATH += $$PWD/3rdparty/log4clib/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/libhash/lib/release/ -lhl++
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/libhash/lib/debug/ -lhl++
else:unix: LIBS += -L$$PWD/3rdparty/libhash/lib/ -lhl++

INCLUDEPATH += $$PWD/3rdparty/libhash/include
DEPENDPATH += $$PWD/3rdparty/libhash/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/libhash/lib/release/libhl++.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/libhash/lib/debug/libhl++.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/libhash/lib/release/hl++.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/3rdparty/libhash/lib/debug/hl++.lib
else:unix: PRE_TARGETDEPS += $$PWD/3rdparty/libhash/lib/libhl++.a

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/libxml2/lib/release/ -lxml2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/libxml2/lib/debug/ -lxml2
else:unix: LIBS += -L$$PWD/3rdparty/libxml2/lib/ -lxml2

INCLUDEPATH += $$PWD/3rdparty/libxml2/include/libxml2
DEPENDPATH += $$PWD/3rdparty/libxml2/include/libxml2
