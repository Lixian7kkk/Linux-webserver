#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>
//线程同步机制封装类


//互斥锁类，多个线程操作一个请求队列所需，请求队列中存放的就是要操作的描述符
class locker {
public:
    locker() {
        if(pthread_mutex_init(&m_mutex, NULL) != 0) {
            throw std::exception();
        }
    }
    ~locker() {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t *get() {
        return &(m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};
//条件变量类，在主线程中使用，通知子线程有描述符需要处理，唤醒它们去竞争，竞争待处理队列（存放待处理描述符的）的锁
class cond {
public:
    cond() {
        if(pthread_cond_init(&m_cond, NULL) != 0) {
            throw std::exception();
        }
    }
    ~cond() {
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *mutex) {//阻塞一个进程
        return pthread_cond_wait(&m_cond, mutex) == 0;
    }
    bool timedwait(pthread_mutex_t *mutex, struct timespec t) {
        return pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
    }
    bool signal() {//唤醒一个进程
        return pthread_cond_signal(&m_cond) == 0; 
    }
    bool broadcast() {//唤醒所有进程
        return pthread_cond_broadcast(&m_cond) == 0;
    }
private:
    pthread_cond_t m_cond;
};
//信号量类，需要操作的描述符的个数
class sem {
public:
    sem() {
        if(sem_init(&m_sem, 0, 0) != 0) {
            throw std::exception();
        }
    }
    sem(int num) {
        if(sem_init(&m_sem, 0, num) != 0) {
            throw std::exception();
        }
    }
    ~sem() {
        sem_destroy(&m_sem);
    }
    //等待信号量
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }
    //增加信号量
    bool post() {
        return sem_post(&m_sem) == 0;
    }
private:
    sem_t m_sem;
};

#endif