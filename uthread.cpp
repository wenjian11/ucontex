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

// �������л���ָ��ID��Э������
void uthread_resume(schedule_t &schedule , int id)
{   
    // ���ID���Ϸ���ֱ�ӷ���
    if(id < 0 || id >= schedule.max_index){
        return;
    }
    // ȡ����Ҫ���е�Э��
    uthread_t *t = &(schedule.threads[id]);
    // ���Э�̵�״̬�ǹ���״̬
    if (t->state == SUSPEND) {
        // ����t-ctx�в�û�б�����һ�������ĵ���Ϣ������ִ�����֮���ɵ�����
        // ���л��Լ�����Э��
        swapcontext(&(schedule.main),&(t->ctx));
    }
}

void uthread_yield(schedule_t &schedule)
{
    // �������������Э����������
    if(schedule.running_thread != -1 ){
        // ȡ���������е�Э��
        uthread_t *t = &(schedule.threads[schedule.running_thread]);
        // ��Э�̹���
        t->state = SUSPEND;
        schedule.running_thread = -1;
        // �л��ص�������Э��
        swapcontext(&(t->ctx),&(schedule.main));
    }
}

// ִ�е��������������е�Э�̵�����
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

// �����������µ�Э�̣���ִ��
int uthread_create(schedule_t &schedule,Fun func,void *arg)
{
    int id = 0;
    // ��һ���Ѿ���������Э�̵�λ��
    for(id = 0; id < schedule.max_index; ++id ){
        if(schedule.threads[id].state == FREE){
            break;
        }
    }
    // ����ҵ�����󣬾ͽ�����Э����Ŀ����1
    if (id == schedule.max_index) {
        schedule.max_index++;
    }
    // ����ֱ�������ȡֵ���о���̫��ȫ�����ӣ�
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
    // ΪЭ�̰󶨺���
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
