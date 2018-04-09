/*
 * AgvThread.h
 *
 *  Created on: 2016-9-1
 *      Author: zxq
 */

#ifndef AGVTHREAD_H_
#define AGVTHREAD_H_

#include <pthread.h>

#include <string>

using namespace std;

class CAgvThread
{
	public:
		CAgvThread(const string &name);

		virtual ~CAgvThread();

		int Start(bool detach);

		virtual void Run();

		int Wait();

		int Stop();

		inline bool IsRunning() {return m_running;}
		inline string &Name() {return m_name;}
	private:
		bool 		m_running;
		pthread_t  	m_thread;
		string 		m_name;
};

#endif /* AGVTHREAD_H_ */
