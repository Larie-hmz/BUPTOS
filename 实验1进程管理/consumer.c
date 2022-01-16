#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>

//the struct of thread
typedef struct _Thread{
  int tid;//thread id
  int delay;// the begin time of read and write operation
  int last;//time that read and write operation lasts
} Thread;

enum {
  s_waiting,
  s_reading,
  s_writing
} state = s_waiting;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t Sig_plate;
sem_t Sig_apple;


time_t start_time;

//Dad process
void* Dad(void* argPtr) {
  Thread thread = *(Thread*)argPtr;
  sleep(thread.delay);

	  printf("[%02.0lfs]Product%d prepare an apple \n", 
	    difftime(time(NULL), start_time), thread.tid
	  );
	  
		sem_wait(&Sig_plate);
	  
		
	  printf("[%02.0lfs]Product%d put the apple on the plate \n", 
	    difftime(time(NULL), start_time), thread.tid
	  );
	  sleep(thread.last);
	  
	  sem_post(&Sig_apple);		



  return NULL;
}

//Daughter process
void* Daughter(void* argPtr) {
  Thread thread = *(Thread*)argPtr;
  sleep(thread.delay);
		
	sem_wait(&Sig_apple);	
	
  printf("[%02.0lfs]Consumer%d take an apple from the plate \n", 
    difftime(time(NULL), start_time), thread.tid
  );
  sleep(thread.last);
	sem_post(&Sig_plate);	
  
	
  printf("[%02.0lfs]Consumer%d eat the apple \n", 
    difftime(time(NULL), start_time), thread.tid  );


  return NULL;
}

int main() {
  const int MAX_THREAD = 100;
  // 读写进程队列
  pthread_t tid[MAX_THREAD];
  Thread thread[MAX_THREAD];
  int tidEnd = 0;
  
  // 初始化信号量
  sem_init(&Sig_plate, 0, 1);
  sem_init(&Sig_apple, 0, 0);

  start_time = time(NULL);

  int arg_tid;
  int arg_delay;
  int arg_last;
  char arg_type;
  //printf("1");
  while(scanf("%d %c%d%d", &arg_tid, &arg_type, &arg_delay, &arg_last) == 4) {
    assert(tidEnd < MAX_THREAD);
    //printf("2");
    if(arg_type == 'P') {
   	 thread[tidEnd].tid = arg_tid;
      thread[tidEnd].delay = arg_delay;
      thread[tidEnd].last = arg_last;
      pthread_create(tid + tidEnd, NULL, Dad, thread + tidEnd);
    } else {
      thread[tidEnd].tid = arg_tid;
      thread[tidEnd].delay = arg_delay;
      thread[tidEnd].last = arg_last;
      pthread_create(tid + tidEnd, NULL, Daughter, thread + tidEnd);
    }
    printf("[%02.0lfs]create process %d\n", difftime(time(NULL), start_time), arg_tid);
    ++tidEnd;
  }
	//printf("3");
   int i; 
  for(i=0; i!=tidEnd; ++i) {
    pthread_join(tid[i], NULL);
  }

   // 销毁信号量
  pthread_mutex_destroy(&mutex);
  sem_destroy(&Sig_plate);
  sem_destroy(&Sig_apple);
}
