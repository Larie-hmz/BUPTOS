#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>

//the struct of thread
typedef struct {
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
sem_t Sig_read;
sem_t Sig_wrt;
int reader_count = 0;
int writer_count = 0;

time_t start_time;

//Readers process
void* Reader(void* argPtr) {
  Thread thread = *(Thread*)argPtr;
  sleep(thread.delay);
  printf("[%02.0lfs]Readers process%d ready to read \n", 
    difftime(time(NULL), start_time), thread.tid
  );

  pthread_mutex_lock(&mutex);
    ++reader_count;
    if(state == s_waiting || state == s_reading) {
      sem_post(&Sig_read);
      state = s_reading;
    }
  pthread_mutex_unlock(&mutex);
  sem_wait(&Sig_read);

//	if(reader_count>1){
//		sem_post(&Sig_read);
//	}

  printf("[%02.0lfs]Readers process%d start to read \n", 
    difftime(time(NULL), start_time), thread.tid
  );

  // read
  sleep(thread.last);

  pthread_mutex_lock(&mutex);
    --reader_count;
    if(reader_count == 0) {
    	//sem_init(&Sig_read, 0, 0);
      if(writer_count != 0) {
        sem_post(&Sig_wrt);
        state = s_writing;
      } else {
        state = s_waiting;
      }
    }
  pthread_mutex_unlock(&mutex);

  printf("[%02.0lfs]Readers process%d end to read \n", 
    difftime(time(NULL), start_time), thread.tid
  );
  return NULL;
}

// writers process
void* Writer(void* argPtr) {
  Thread thread = *(Thread*)argPtr;
  sleep(thread.delay);
  printf("[%02.0lfs]Writers process%d ready to write \n", 
    difftime(time(NULL), start_time), thread.tid
  );
  pthread_mutex_lock(&mutex);
  ++writer_count;
  if(state == s_waiting) {
    sem_post(&Sig_wrt);
    state = s_writing;
  }
  pthread_mutex_unlock(&mutex);

  sem_wait(&Sig_wrt);
  printf("[%02.0lfs]Writers process%d start to write \n", 
    difftime(time(NULL), start_time), thread.tid
  );
  // write
  sleep(thread.last);

  pthread_mutex_lock(&mutex);
    --writer_count;
    if(reader_count != 0) {
		for(int i=0;i<reader_count;i++){
			sem_post(&Sig_read);
		}
	      sem_post(&Sig_read);
      state = s_reading;
    } else if(writer_count != 0) {
      sem_post(&Sig_wrt);
      state = s_writing;
    } else {
      state = s_waiting;
    }
  pthread_mutex_unlock(&mutex);

  printf("[%02.0lfs]Writers process%d end to write \n", 
    difftime(time(NULL), start_time), thread.tid
  );
  return NULL;
}

int main() {
  const int MAX_THREAD = 100;
  // 读写进程队列
  pthread_t tid[MAX_THREAD];
  Thread thread[MAX_THREAD];
  int tidEnd = 0;

  // 初始化信号量
  sem_init(&Sig_read, 0, 0);
  sem_init(&Sig_wrt, 0, 0);

  start_time = time(NULL);

  int arg_tid;
  int arg_delay;
  int arg_last;
  char arg_type;
  while(scanf("%d %c%d%d", &arg_tid, &arg_type, &arg_delay, &arg_last) == 4) {
    assert(tidEnd < MAX_THREAD);
    if(arg_type == 'R') {
      thread[tidEnd].tid = arg_tid;
      thread[tidEnd].delay = arg_delay;
      thread[tidEnd].last = arg_last;
      pthread_create(tid + tidEnd, NULL, Reader, thread + tidEnd);
    } else {
      thread[tidEnd].tid = arg_tid;
      thread[tidEnd].delay = arg_delay;
      thread[tidEnd].last = arg_last;
      pthread_create(tid + tidEnd, NULL, Writer, thread + tidEnd);
    }
    printf("[%02.0lfs]create process %d\n", difftime(time(NULL), start_time), arg_tid);
    ++tidEnd;
  }

   int i; 
  for(i=0; i!=tidEnd; ++i) {
    pthread_join(tid[i], NULL);
  }

   // 销毁信号量
  pthread_mutex_destroy(&mutex);
  sem_destroy(&Sig_read);
  sem_destroy(&Sig_wrt);
}
