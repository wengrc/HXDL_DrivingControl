CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(ForkLift)

SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_PATH "/usr/local/arm/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi")
# specify the cross compiler
SET(CMAKE_C_COMPILER   "${TOOLCHAIN_PATH}/bin/arm-linux-gcc")
SET(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/bin/arm-linux-gcc")

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PATH})

# search for programs in the build host directories (not necessary)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")

SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}")

##############################################################################

SET(CMAKE_BUILD_TYPE Debug)
#SET(CMAKE_BUILD_TYPE Release)

SET(SOURCE_FILES
    comm/AgvHttp.cpp
    comm/AgvHttp.h
    comm/AgvJson.cpp
    comm/AgvJson.h
    comm/AgvSchProtocol.cpp
    comm/AgvSchProtocol.h
    comm/AgvSocketClient.cpp
    comm/AgvSocketClient.h
    comm/AgvSocketServer.cpp
    comm/AgvSocketServer.h
    controler/AgvCar.cpp
    controler/AgvCar.h
    controler/AgvRoutePlanner.cpp
    controler/AgvRoutePlanner.h
    controler/AgvScheduler.cpp
    controler/AgvScheduler.h
    devices/canbus/AgvControler.cpp 
    devices/canbus/AgvControler.h
    devices/canbus/AgvCanOperator.cpp
    devices/canbus/AgvCanOperator.h
    devices/canbus/AgvDownloadBin.cpp
    devices/canbus/AgvDownloadBin.h
    devices/canbus/AgvSocketCan.cpp
    devices/canbus/AgvSocketCan.h
    devices/canbus/AgvSensorManager.cpp
    devices/canbus/AgvSensorManager.h
    devices/canbus/AgvSensor.cpp
    devices/canbus/AgvSensor.h
    devices/network/AgvWifi.cpp
    devices/network/AgvWifi.h
    event/AgvEvent.h
    event/AgvEventHelper.cpp
    event/AgvEventHelper.h
    main/AgvMainLoop.cpp
    main/AgvMainLoop.h
    main/AgvSetting.cpp
    main/AgvSetting.h
    utils/AgvCheckSum.cpp
    utils/AgvCheckSum.h
    utils/AgvLogs.cpp
    utils/AgvLogs.h
    utils/AgvThread.cpp
    utils/AgvThread.h
    utils/AgvUtils.cpp
    utils/AgvUtils.h
    utils/AgvXml.cpp
    utils/AgvXml.h
    utils/AgvXmlSetting.cpp
    utils/AgvXmlSetting.h
    AgvPublic.h
    main.cpp )


INCLUDE_DIRECTORIES(
        3rdparty/libcurl/include
        3rdparty/libhash/include
        3rdparty/libjsonc/include
        3rdparty/libxml2/include/libxml2
        3rdparty/log4clib/include
)

LINK_DIRECTORIES(
        3rdparty/libcurl/lib
        3rdparty/libhash/lib
        3rdparty/libjsonc/lib
        3rdparty/libxml2/lib
        3rdparty/log4clib/lib
)

ADD_EXECUTABLE(ForkLift ${SOURCE_FILES})


TARGET_LINK_LIBRARIES(
        ForkLift
        curl
        hl++
        json
        xml2
        log4cplus
)
