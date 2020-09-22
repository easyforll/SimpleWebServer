#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<list>
#include<cstdio>
#include<exception>
#include<pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class threadpool{
public:
    threadpool(const threadpool<T>&);//addcode:禁止复制拷贝
    const threadPool& operator=(const threadpool<T>&);//
    threadpool(int actor_model,connection_pool * connpool,int thread_number =8,int max_request=10000);
    ~threadpool(T* request ,int state);
    bool append_p(T* request);
private:
    //use static to be invoked conveniently by thread entrance function?
    static void * worker(void* arg);
    void run();

private:
    int m_thread_number;        //thread number
    int m_max_requests;         //maxrequest in queue;
    pthread_t* m_threads;       //thread array;the size is number
    std::list<T*> m_workqueue;       //queue
    locker m_queuelocker;       //the mutex of queue-
    sem m_queuestat;            //whether there is task to be delt
    connection_pool* m_connPool;//data base
    int m_actor_model           //model exchange
};

template<typename T>
threadpool<T>::threadpool(int actor_model,connection_pool * connPool,int thread_number,int max_requests):m_actor_model(actor_model),m_thread_number(thread_number),m_max_requests(max_requests),m_threads(NULL),m_connPool(connPool)
{
    if(thread_number<=0||max_requests<=0) 
        throw std::exception();
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads)
        throw std::exception();
    for(int i=0;i<thread_number;++i){
        if(pthread_create(m_threads+i,NULL,worker,this)!=0){
            delete[] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i])){
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template<typename T>
threadpool<T>::~threadpool(){
    delete[] m_threads;
}
template<typename T>
bool threadpool<T>::append(T* request,int state){
    m_queuelocker.lock();               //when append new request to queue,need mutex to avoid competition visit of queue;
    if(m_workqueue.size()>=m_max_requests){
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state;         //bad code! typename T need member state!
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();                 //sem is used here! still cannot understand the function of sem
    return true;
}
template <typename T>
bool threadpool<T>::append_p(T* request){
    m_queuelocker.lock();
    if(m_workqueue.size()>=m_max_requests){
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}
template<typename T>
void* threadpool<T>::worker(void* arg){
    threadpool* pool = (threadpool *) arg;
    pool->run();
    return pool;
}
template<typename T>
void* threadpool<T>::run(){
    while(true){
        m_queuestat.wait(); //correspond to m_queuestat, just like producer(append) and consumer(run)
        m_queuelocker.lock();
        if(m_workqueue.empty()){
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request)
            continue;
        if(1 == m_actor_model){
            if(0 == request -> m_state){        //coupling: the request must have the member state
                if(request->read_once()){       //coupling: the request must have the member function read_once
                    request->improv = 1;        //???
                    connectionRAII mysqlcon(&request->mysql, m_connPool);
                    request->process();
                }else{
                    request->improv =1;
                    request->timer_flag =1;
                }

            }else{
                if(request->write()){
                    request -> improv = 1 ;
                }else{
                    request ->improv = 1;
                    request ->timer_flag = 1;
                }
            }
        }else{
            connectionRAII mysqlcon(&request->mysql, m_connPool);
            request->process();
        }
    }
}

#endif