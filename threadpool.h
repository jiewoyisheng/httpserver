//threadpool.h
#ifndef THREADLPOOL_H_
#define THREADLPOOL_H_

#include<list>
#include<pthread.h>
#include<cstdio>
#include<exception>
using namespace std;

template<typename T >
class threadpool
{
public:
	threadpool(int thread_number = 8, int max_requests = 10000);
	~threadpool();
	bool append(T* request);
private:
	bool run();
	static void* worker( void* arg );
	
private:
	int m_thread_number;
	int m_max_requests;
	list<T*> m_workqueue;
	locker m_queuelocker;
	sem m_queuestat;
	bool m_stop;
	pthread_t* m_threads; 

};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests):
						m_thread_number(thread_number),m_max_requests(max_requests),
						m_stop(false),m_threads(NULL)
{
	if(thread_number<0&& max_requests < 0 )
	{
		throw exception();
	}
	
	m_threads = new pthread_t[m_max_requests];
	
	for(int i=0;i<m_max_requests;i++)
	{
		if(pthread_creat(m_threads+i,NULL,worker,this) != 0 )
		{
			delete [] m_threads;
			throw exception();
		}
		if(pthread_detach(m_threads[i]) != 0 )
		{
			delete [] m_threads;
			throw exception();
		}
	}
		
}
							
	
template<typename T>
threadpool<T>::~threadpool()
{
	delete [] m_threads;
	m_stop = true;
	
}	

template<typename T>
bool threadpool<T>::append(T* request)
{
	m_queuelocker.lock();
	if(m_workqueue.size()>m_max_requests)
	{
		m_queuelocker.unlock();
		return false;
	}
	m_workqueue.push_back(request);
	m_queuelocker.unlock();
	m_queuestat.post();
	return true;
}


template<tpyename T>
void threadpool<T>::run()
{
	while(!stop)
	{
		m_queuestat.wait();
		m_queuelocker.lock();
		if(m_workqueue.empty())
		{
			m_queuelocker.unlock();
			continue;
		}
		T* request = m_workqueue.front();
		m_workqueue.pop_front();
		m_queuelocker.unlock();
		if(! request )
		{
			continue;
		}
		request->process();
	}
}


template<typename T>
void * threadpool<T>::worker(void * arg)
{
	threadpool* pool = (threadpool*)arg;
	arg->run();
	return pool;
}




























#endif