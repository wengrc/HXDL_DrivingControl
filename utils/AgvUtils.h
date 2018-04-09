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
         * ��buffer�������תΪ��ֵ
         * @param data  buffer����ָ��
         * @param bytes Ҫת������ֵ�ֽ���
         * @return ת�������ֵ
         */
        static int Buffer2Bin(const unsigned char *data, int bytes);


        /**
         * ����ֵת����buffer��
         * @param data  ��ֵ
         * @param bytes ��ֵ�ֽ���
         * @param buf   ������ݵ�ָ��
         * @return ת�����ֽ���
         */

        static int Bin2Buffer(int data, int bytes, unsigned char *buf);

        /**
         * ��ȡĳ�����̵�pid��
         * @param process ��������
         * @return
         *      - >0  �ý��̵�PID��
         *      - <=0 ��ȡʧ��
         */
        static int GetPid(const char *process);


        /**
         * ɱ��ĳ������
         * @param process ��������
         * @return
         *      - 0: ִ�гɹ�
         *      - <0: ִ��ʧ��
         */
        static int KillProcess(const char *process, bool wait);

        /**
         * ��鵱ǰ��ǰ�������Ƿ����Ip��ַ
         * @param ip Ҫ�жϵ�IP��ַ
         * @param isExsiting �����Ƿ���ڵ�ָ��
         * @return
         *      - 0: ִ�гɹ�����������isExisting��
         *      - <0: ִ��ʧ��
         */
        static int CheckIpAddress(const char *ip, bool *isExsiting);

        /**
         * ��ȡ�����豸��IP��ַ
         * @param ifname �����豸����
         * @param ip    ���IP��ַ��ָ��
         * @param size  ���IP��ַ�ĳ���
         * @return
         *      - 0: ִ�гɹ�����������isExisting��
         *      - <0: ִ��ʧ��
         */
        static int GetIpAddress(const char *ifname, char *ip, unsigned int size);

        /**
         * ��������豸��״̬
         * @param ifname �����豸����
         * @return
         *      - 1�������豸�Ѿ�������δ����
         *      - 0: ����������
         *      - <0: �����豸����
         */
        static int CheckNetStatus(const char *ifname);

        /**
         * ���������豸��IP��ַ
         * @param ifname �����豸����
         * @param ip Ҫ���õ�IP��ַ
         * @return
         *      - 0: ִ�гɹ�
         *      - ����: ִ��ʧ��
         */
        static int SetIpAddress(const char *ifname, const char *ip);

        /**
         * �����豸DHCP��ʽ����ȡIP��ַ
         * @param ifname �����豸����
         * @param times  ���Դ�����һ��3�룬 Ϊ0ʱ�ź�̨���У�Ϊ-1ʱ��һֱ�ȴ�����ȡΪֹ
         * @return
         *      - 0: ��ȡDHCP�ɹ�
         *      - ��������ȡʧ��
         */
        static int StartDhcp(const char *ifname, int times);
};

#endif /* CAGVUTILS_H_ */
