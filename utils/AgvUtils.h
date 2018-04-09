/*
 * AgvUtils.h
 *
 *  Created on: 2016-9-1
 *      Author: zxq
 */

#ifndef CAGVUTILS_H_
#define CAGVUTILS_H_

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG

#define DEBUG_INFO(pre,fmt, ...) fprintf(stdout,"["pre"] " fmt "\n", ##__VA_ARGS__ )
#define DEBUG_ERROR(pre,fmt, ...) fprintf(stderr,"["pre"] " fmt "{%s} in %s :%d""\n", ##__VA_ARGS__, __FUNCTION__, __FILE__, __LINE__ )
#else

#define DEBUG_INFO(pre,fmt, ...)
#define DEBUG_ERROR(pre,fmt, ...) fprintf(stderr,"["pre"] " fmt "\n", ##__VA_ARGS__ )

#endif

class CAgvUtils
{
    public:
        CAgvUtils();
        virtual ~CAgvUtils();

        /**
         * 将buffer里的数据转为数值
         * @param data  buffer数据指针
         * @param bytes 要转换的数值字节数
         * @return 转换后的数值
         */
        static int Buffer2Bin(const unsigned char *data, int bytes);


        /**
         * 将数值转换到buffer中
         * @param data  数值
         * @param bytes 数值字节数
         * @param buf   存放数据的指针
         * @return 转换的字节数
         */

        static int Bin2Buffer(int data, int bytes, unsigned char *buf);

        /**
         * 获取某个进程的pid号
         * @param process 进程名称
         * @return
         *      - >0  该进程的PID号
         *      - <=0 获取失败
         */
        static int GetPid(const char *process);


        /**
         * 杀死某个进程
         * @param process 进程名称
         * @return
         *      - 0: 执行成功
         *      - <0: 执行失败
         */
        static int KillProcess(const char *process, bool wait);

        /**
         * 检查当前当前网络中是否存在Ip地址
         * @param ip 要判断的IP地址
         * @param isExsiting 返回是否存在的指针
         * @return
         *      - 0: 执行成功，结果存放中isExisting中
         *      - <0: 执行失败
         */
        static int CheckIpAddress(const char *ip, bool *isExsiting);

        /**
         * 获取网络设备的IP地址
         * @param ifname 网络设备名称
         * @param ip    存放IP地址的指针
         * @param size  存放IP地址的长度
         * @return
         *      - 0: 执行成功，结果存放中isExisting中
         *      - <0: 执行失败
         */
        static int GetIpAddress(const char *ifname, char *ip, unsigned int size);

        /**
         * 检测网络设备的状态
         * @param ifname 网络设备名称
         * @return
         *      - 1：网络设备已就绪，但未连接
         *      - 0: 网络已连接
         *      - <0: 网络设备错误
         */
        static int CheckNetStatus(const char *ifname);

        /**
         * 设置网络设备的IP地址
         * @param ifname 网络设备名称
         * @param ip 要设置的IP地址
         * @return
         *      - 0: 执行成功
         *      - 其他: 执行失败
         */
        static int SetIpAddress(const char *ifname, const char *ip);

        /**
         * 网络设备DHCP方式来获取IP地址
         * @param ifname 网络设备名称
         * @param times  重试次数，一次3秒， 为0时放后台运行，为-1时则一直等待到获取为止
         * @return
         *      - 0: 获取DHCP成功
         *      - 其他：获取失败
         */
        static int StartDhcp(const char *ifname, int times);
};

#endif /* CAGVUTILS_H_ */
