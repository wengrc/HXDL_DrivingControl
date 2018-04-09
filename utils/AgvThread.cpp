/*
 * AgvThread.cpp
 *
 *  Created on: 2016-9-1
 *      Author: zxq
 */
#include "AgvThread.h"
#include "../AgvPublic.h"

void *doThreadJob(void *arg)
{
	CAgvThread *thread = (CAgvThread*)arg;
	if (arg != NULL)
	{
		thread->Run();
	}
	pthread_exit(NULL);

	return NULL;
}


CAgvThread::CAgvThread(const string &name)
{
	m_name = name;
	m_thread = 0;
	m_running = false;
}

CAgvThread::~CAgvThread()
{
	Stop();
}

int CAgvThread::Start(bool detach)
{
	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	if (detach)
	{
		pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED);
	}

	int ret = pthread_create(&m_thread, &thread_attr, doThreadJob, this);

	pthread_attr_destroy(&thread_attr);

	if (ret != 0)
	{
		printf("Create Thread %s failed!:%s\n", m_name.c_str(), strerror(errno));
	}
	m_running = true;
	return ret;
}


void CAgvThread::Run()
{

}


int CAgvThread::Wait()
{
	if (m_thread <= 0)
	{
		return -1;
	}

	if (!m_running)
	{
		return 0;
	}

	int ret = pthread_join(m_thread, NULL);

	m_running = false;

	return ret;
}

int CAgvThread::Stop()
{
	if (m_thread <= 0)
	{
		return -1;
	}

	if (!m_running)
	{
		return 0;
	}

	int ret = pthread_cancel(m_thread);
	m_running = false;
	return ret;

}
