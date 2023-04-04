#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <list>
#include "locker.h"
#include <cstdio>

//线程池类，定义成模板类为了代码的复用，模板参数T是任务类
template<class T>
class threadpool
{
private:
    static void *worker(void *arg);//...
    void run();
private:
    /* data */
    //线程数量
    int m_thread_number;
    //线程池数组，大小为m_thread_number，其中每个元素是子线程ID
    pthread_t *m_threads;
    //请求队列中最多允许的等待请求处理的数量（有多少个描述符需要读写操作了）
    int m_max_requests;
    //请求队列（list实现）
    std::list<T *> m_workqueue;//需要执行操作的描述符
    //互斥锁（此处用于互斥的去操作请求队列）
    locker m_queuelocker;
    //信号量用于判断是否有任务需要处理（需要进行读写操作的描述符的个数）。。。
    sem m_queuestat;
    //是否结束线程
    bool m_stop;
public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T *request);
};
template<class T>
threadpool<T>::threadpool(int thread_number, int max_requests) : 
    m_thread_number(thread_number), m_max_requests(max_requests),
    m_stop(false), m_threads(NULL) {
    if((thread_number <= 0) || max_requests <= 0) {
        throw std::exception();
    }
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads) {
        throw std::exception();
    }
    //创建thread_number个线程，并设置为线程脱离使其能够自动回收资源
    for(int i = 0; i < thread_number; ++i)
    {
        printf("create the %dth thread\n", i);
        if(pthread_create(m_threads + i, NULL, worker, this) != 0) {//第一个参数是传出参数，将创建的子线程ID写入该变量，此处理解是m_threads + i指针指向子线程ID
            delete [] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i])) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}   
template<class T>
threadpool<T>::~threadpool() {
    delete [] m_threads;
    m_stop = true;
}
template<class T>
bool threadpool<T>::append(T *request) {
    m_queuelocker.lock();//操作等待队列要上锁
    if(m_workqueue.size() >= m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);//添加进等待队列
    m_queuelocker.unlock();
    m_queuestat.post();//需要处理的描述符+1
    return true;
}
template<class T>
void *threadpool<T>::worker(void *arg) {
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}
template<class T>
void threadpool<T>::run() {
    while(!m_stop) {
        m_queuestat.wait();//处理一个描述符，并使其值-1
        m_queuelocker.lock();
        if(m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        T *request = m_workqueue.front();
        m_workqueue.pop_front();//拿出一个描述符并进行操作
        m_queuelocker.unlock();
        if(!request) {
            continue;
        }
        request->process();//执行对描述符的相关操作
    }
}
#endif