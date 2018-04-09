/*
 * AgvWifi.h
 *
 *  Created on: 2016-8-31
 *      Author: zxq
 */

#ifndef AGVWIFI_H_
#define AGVWIFI_H_

#include <string>
#include <vector>

using namespace std;

class CAgvWifi
{

	public:
		enum Mode{
			StandMode,
			ApMode
		};

		enum Status{
			DevOff,
			DevReady,
			Connecting,
			Connected,
		};

	public:
        CAgvWifi();

		virtual ~CAgvWifi();

		int PowerOn(int mode, string ifname = "wlan0");

		bool CheckUsbWifi();

		int ChangeMode(int mode);

		inline int CurrentMode() {return currentMode;}

		int GetAPList(vector<string> &aplist);

		int CheckApIsExist(const char *apName);

		int SetApConnection(const char *apName, const char *pwd);

		int ConnectToAp(int timeout);

		bool isConnected();

		bool CheckConnectStatus();

		int SetIpAddress(const char *ip, bool dhcp);

		int Disconnect();

		int PowerOff();

		static bool checkUsbDeviceIsExist(const char *pidvid);

		inline const string &DevName() {return devName;}
	private:

		bool haveUsbWifi;
		int currentMode;
		int status;
		string devName;
		string currentAp;
		string currentPasswod;

};

#endif /* AGVWIFI_H_ */
