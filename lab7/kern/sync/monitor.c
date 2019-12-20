#include <stdio.h>
#include <monitor.h>
#include <kmalloc.h>
#include <assert.h>


// Initialize monitor.
void     
monitor_init (monitor_t * mtp, size_t num_cv) {
    int i;
    assert(num_cv>0);
    mtp->next_count = 0;
    mtp->cv = NULL;
    sem_init(&(mtp->mutex), 1); //unlocked
    sem_init(&(mtp->next), 0);
    mtp->cv =(condvar_t *) kmalloc(sizeof(condvar_t)*num_cv);
    assert(mtp->cv!=NULL);
    for(i=0; i<num_cv; i++){
        mtp->cv[i].count=0;
        sem_init(&(mtp->cv[i].sem),0);
        mtp->cv[i].owner=mtp;
    }
}

// Unlock one of threads waiting on the condition variable. 
void 
cond_signal (condvar_t *cvp) {
   //LAB7 EXERCISE1: YOUR CODE
   cprintf("cond_signal begin: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
    //判断当前的条件变量的等待队列上是否有正在等待的进程，如果没有则不需要进行任何操作；
    //如果有正在等待的进程
	if(cvp->count>0) {
        //将其中一个唤醒
        up(&(cvp->sem));
        //所属管程的next计数加1，表示当前进程会被等待者堵塞
        cvp->owner->next_count ++;
        //阻塞，等待条件同步
        down(&(cvp->owner->next));
        //当前进程被唤醒，恢复next上的等待进程计数
        cvp->owner->next_count --;
      }
   cprintf("cond_signal end: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}

// Suspend calling thread on a condition variable waiting for condition Atomically unlocks 
// mutex and suspends calling thread on conditional variable after waking up locks mutex. Notice: mp is mutex semaphore for monitor's procedures
void
cond_wait (condvar_t *cvp) {
    //LAB7 EXERCISE1: YOUR CODE
    cprintf("cond_wait begin:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
    cvp->count++;// 修改等待在条件变量的等待队列上的进程计数
    //当管程的 next_count 大于0，说明有进程睡在了 signal 操作上我们将其唤醒
    if (cvp->owner->next_count > 0)
    { // 释放锁
        up(&cvp->owner->next);
    }
    else  //当前没有进程睡在 signal操作数 只需要释放互斥体
    {
        up(&cvp->owner->mutex);
    }
    //将自身阻塞，等待条件变量的条件为真，被唤醒后将条件不成立而睡眠的进程计数减1
    down(&cvp->sem); // 将自己等待在条件变量上
    cvp->count --; // 被唤醒，修正等待队列上的进程计数
    cprintf("cond_wait end:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}
