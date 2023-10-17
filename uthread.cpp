/**
* @file  uthread.cpp
* @author chenxueyou
* @version 0.1
* @brief   :A asymmetric coroutine library for C++
* History
*      1. Date: 2014-12-12 
*          Author: chenxueyou
*          Modification: this file was created 
*/

#ifndef MY_UTHREAD_CPP
#define MY_UTHREAD_CPP


#include "uthread.h"
//#include <stdio.h>

// 调度器切换到指定ID的协程运行
void uthread_resume(schedule_t &schedule , int id)
{   
    // 如果ID不合法，直接返回
    if(id < 0 || id >= schedule.max_index){
        return;
    }
    // 取出将要运行的协程
    uthread_t *t = &(schedule.threads[id]);
    // 如果协程的状态是挂起状态
    if (t->state == SUSPEND) {
        // 这里t-ctx中并没有保存下一个上下文的信息，而是执行完毕之后，由调度器
        // 再切回自己的主协程
        swapcontext(&(schedule.main),&(t->ctx));
    }
}

void uthread_yield(schedule_t &schedule)
{
    // 如果调度器中有协程正在运行
    if(schedule.running_thread != -1 ){
        // 取出正在运行的协程
        uthread_t *t = &(schedule.threads[schedule.running_thread]);
        // 将协程挂起
        t->state = SUSPEND;
        schedule.running_thread = -1;
        // 切换回调度器主协程
        swapcontext(&(t->ctx),&(schedule.main));
    }
}

// 执行调度器中正在运行的协程的任务
void uthread_body(schedule_t *ps)
{
    int id = ps->running_thread;

    if(id != -1){
        uthread_t *t = &(ps->threads[id]);

        t->func(t->arg);

        t->state = FREE;
        
        ps->running_thread = -1;
    }
}

// 调度器创建新的协程，并执行
int uthread_create(schedule_t &schedule,Fun func,void *arg)
{
    int id = 0;
    // 找一个已经完成任务的协程的位置
    for(id = 0; id < schedule.max_index; ++id ){
        if(schedule.threads[id].state == FREE){
            break;
        }
    }
    // 如果找到了最后，就将最大的协程数目增加1
    if (id == schedule.max_index) {
        schedule.max_index++;
    }
    // 这里直接再完后取值，感觉不太安全的样子？
    uthread_t *t = &(schedule.threads[id]);

    t->state = RUNNABLE;
    t->func = func;
    t->arg = arg;

    getcontext(&(t->ctx));
    
    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = DEFAULT_STACK_SZIE;
    t->ctx.uc_stack.ss_flags = 0;
    t->ctx.uc_link = &(schedule.main);
    schedule.running_thread = id;
    // 为协程绑定函数
    makecontext(&(t->ctx),(void (*)(void))(uthread_body),1,&schedule);
    swapcontext(&(schedule.main), &(t->ctx));
    
    return id;
}

int schedule_finished(const schedule_t &schedule)
{
    if (schedule.running_thread != -1){
        return 0;
    }else{
        for(int i = 0; i < schedule.max_index; ++i){
            if(schedule.threads[i].state != FREE){
                return 0;
            }
        }
    }

    return 1;
}

#endif
