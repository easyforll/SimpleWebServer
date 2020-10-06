#ifndef LOCKER_H
#define LOCKER_H
#include<exception>
#include<pthread.h>
#include<semaphore.h>


/*
    pack thread synchronization class:
    1.semqphore
    2.mutex exclusion
    3.condition variable
*/
//RAII 
//Notice: when packing the system function ,keep the consistent sematics between the packed func and initial func
//Notice: construct function can throw an exception but destroy function needn't ,why?
class sem{
    private:
        sem_t m_sem;
    public:
        sem(){
            //initialize the semaphore
            if(sem_init(&m_sem,0,0)!=0){
                throw std::exception();
            }
        }
        sem(int num){
            if(sem_init(&m_sem,0,num)!=0){
                throw std::exception();
            }
        }
        ~sem(){
            sem_destroy(&m_sem);
        }
        bool wait(){
            return sem_wait(&m_sem)==0;
        }
        bool post(){
            return sem_post(&m_sem)==0;
        }
};
class locker
{
    private:
        pthread_mutex_t m_mutex;
    public:
        locker(){
            if(pthread_mutex_init(&m_mutex,NULL)!=0){
                throw std::exception();
            }
        }
        ~locker(){
            pthread_mutex_destroy(&m_mutex);
        }
    //notice: the execution of the function
        bool lock(){
            return pthread_mutex_lock(&m_mutex)==0;
        }
        bool unlock(){
            return pthread_mutex_unlock(&m_mutex)==0;
        }
    //?? what does this mean?
        pthread_mutex_t *get(){
            return &m_mutex;
        }
};

class cond{
    private:
        pthread_cond_t m_cond;
    public:
        cond(){
            if(pthread_cond_init(&m_cond,NULL)){
                throw std::exception();
            }
        }
        ~cond(){
            pthread_cond_destroy(&m_cond);
        }
        bool wait(pthread_mutex_t* mutex){
            int ret =0;
            ret = pthread_cond_wait(&m_cond,mutex);
            return ret==0;
        }
        bool timewait(pthread_mutex_t* m_mutex,struct timespec t){
            int ret=0;
            ret = pthread_cond_timedwait(&m_cond,m_mutex,&t);
            return ret==0;
        }
        bool signal()
        {
            return pthread_cond_signal(&m_cond)==0;
        }
        bool broadcast(){
            return pthread_cond_broadcast(&m_cond)==0;
        }
};

#endif
