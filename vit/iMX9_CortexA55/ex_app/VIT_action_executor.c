/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "VIT_action_executor.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/queue.h>
#include <stdlib.h>

#ifdef RUN_ON_OTHER_THREAD
static void VIT_init_custom()
{
  // Put your code here
}

static void VIT_call_custom(int cmd)
{
  pid_t tid = syscall(SYS_gettid);
  LOG("  %d Processing command: %d\n", tid, cmd );
  // Put your code here


  LOG("  %d done processing command: %d\n", tid, cmd );
}


/*****************************************************************************/
/*             CODE TO EXEC IN OTHER THREAD                                  */
/*****************************************************************************/
static volatile int running_ = 1;
static pthread_t thread_;
static pthread_cond_t cond_ = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t  mutex_ = PTHREAD_MUTEX_INITIALIZER;

TAILQ_HEAD(tailhead, elem) head_;
volatile int commads_queued_ = 0;

struct elem {
  int cmd;
  TAILQ_ENTRY(elem) elems;
};

void add_to_queue(int c) 
{
  struct elem *elem;
  elem = malloc(sizeof(struct elem));
  if (elem) {
    elem->cmd = c;
    commads_queued_++;
  }
  TAILQ_INSERT_HEAD(&head_, elem, elems);
}

int get_from_queue() 
{
  int ret = 0;
  struct elem *elem = TAILQ_LAST(&head_, tailhead);
  ret = elem->cmd;
  TAILQ_REMOVE(&head_, elem, elems);
  commads_queued_--;
  free(elem);
  return ret;
}

static void* VIT_run_thread_routine(void* args)
{
  INFO("VIT_run_thread_routine\n");
  // run a new thread
  while (running_)
  {
    int cmd = 0;

    // wait on cond variable if no new command arrived
    while (commads_queued_ == 0) {
      pthread_cond_wait(&cond_, &mutex_);
    }
    // get value
    cmd = get_from_queue();

    // exit critical section
    pthread_mutex_unlock(&mutex_);

    // exec HERE outside critical section
    VIT_call_custom(cmd);

  }
  INFO("VIT_run_thread_routine...exiting\n");
  return 0;
}

int VIT_send_cmd(int cmd)
{
  // lock mutex
  pthread_mutex_lock(&mutex_);
  //update values
  add_to_queue(cmd);
  // broadcast cond
  pthread_cond_broadcast(&cond_);
  // exit critical section
  pthread_mutex_unlock(&mutex_);
  return 0;
}

int VIT_run_thread()
{
  INFO("run thread\n");
  VIT_init_custom();

  TAILQ_INIT(&head_);
  pthread_cond_init(&cond_, NULL);

  return pthread_create(&thread_, NULL, VIT_run_thread_routine, NULL);
}

int VIT_stop_thread()
{
  INFO("stop thread\n");
  running_ = 0;
  // push fake comamnd to wake up cond
  VIT_send_cmd(-1);

  // wait for thread to exit
  pthread_join(thread_, NULL);

  pthread_cond_destroy(&cond_);
  return 0;
}
#else
int VIT_run_thread() { return 0; }
int VIT_stop_thread(){ return 0; }
int VIT_send_cmd(int cmd){ return 0; }
#endif
