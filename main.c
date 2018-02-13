#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "libperf.h"
#include <pthread.h>

#define THREAD_SIZE 4

int ini_pl[100000] = {0};
int cur_pl[100000] = {0};
int new_pl[100000] = {0};
int T_INIT_PID_CNT;
int T_CUR_PID_CNT;

void set_new_pl(int init_cnt, int cur_cnt){
	
  int ip, cp, np;
  int iter = 0;
  ip = cp = np = 0;
  while(ip != init_cnt && cp != cur_cnt){
    if(ini_pl[ip] == cur_pl[cp]){
	ip++;
	cp++;
    }
    else if(ini_pl[ip] < cur_pl[cp])
	ip++;
    else if(ini_pl[ip] > cur_pl[cp])
	new_pl[np++] = cur_pl[cp++]; 
  }
  if(cp < cur_cnt)
    for(iter = cp; iter < cur_cnt; iter++){
    new_pl[np] = cur_pl[iter];
    np++;
  }
}

int get_pid_cnt(int set_pl[]) {
  
  int cnt = 0;
  int i = 0;
  DIR *dir;
  int pid; 
  struct dirent *entry;   
  struct stat fileStat;
  dir = opendir("/proc");
  for(i = 0; i < 100000; i++) set_pl[i] = 0;
  while ((entry = readdir(dir)) != NULL) {  
    lstat(entry->d_name, &fileStat);
//  printf("dname: %s, %d, ISDIR : %d\n", entry->d_name, fileStat.st_mode, S_ISDIR(fileStat.st_mode));
//  if (S_ISDIR(fileStat.st_mode)) 
//       continue; 
    pid = atoi(entry->d_name); 
//  	printf("pid : %d, cnt : %d\n", pid, cnt);
    if (pid <= 0) continue;    
//  	sprintf(tempPath, "/proc/%d/cmdline", pid);
    set_pl[cnt++] = pid;
  }
  closedir(dir);
  return cnt;
}

void *cont_counter(int pid){

    int LIBPERF_COUNT_ITER = 0;
    // first, input to the pid to check
 
    //printf("last PID : %d\n", pid);
    struct libperf_data *pd = libperf_initialize(pid, -1);
    for(LIBPERF_COUNT_ITER = 0; LIBPERF_COUNT_ITER < 14; LIBPERF_COUNT_ITER++){
    //Num of counters is 14
      libperf_enablecounter(pd, LIBPERF_COUNT_ITER);
    }
    for(LIBPERF_COUNT_ITER = 0; LIBPERF_COUNT_ITER < 14; LIBPERF_COUNT_ITER++){
      uint64_t counter = libperf_readcounter(pd, LIBPERF_COUNT_ITER);
    }	
    sleep(1);
  //  	usleep(100000);
    for(LIBPERF_COUNT_ITER = 0; LIBPERF_COUNT_ITER < 14; LIBPERF_COUNT_ITER++){
      libperf_disablecounter(pd, LIBPERF_COUNT_ITER);
    }	
    FILE * log = libperf_getlogger(pd);
    fprintf(log, "custom log message\n");
    libperf_finalize(pd, 0);
    
    //unlock a pthread
}

int main() {
  
  pid_t CUR_PID = getpid();
  int CUR_PID_CNT = 0;
  pthread_t p_thread[THREAD_SIZE];
  int thr_id;
  int status;
  

  int INIT_PID_CNT = get_pid_cnt(ini_pl);
  T_INIT_PID_CNT = INIT_PID_CNT;
  CUR_PID_CNT = 0;
  while(1){
    int ENT_ITER = 0;
    CUR_PID_CNT = 0;

  	//printf("\nChecker Process ID : %d\n", CUR_PID);
  
    CUR_PID_CNT = get_pid_cnt(cur_pl);
    T_CUR_PID_CNT = CUR_PID_CNT;

//  	printf("\ninit : %d\n", INIT_PID_CNT);
//  	printf("\nbefore Process >> CUR_PID_CNT : %d\n", CUR_PID_CNT);
    if(INIT_PID_CNT != CUR_PID_CNT){
      int NEW_PIDS = CUR_PID_CNT - INIT_PID_CNT;
      
      set_new_pl(INIT_PID_CNT, CUR_PID_CNT);

      int TRANS_ITER = 0;
      for(TRANS_ITER = 0; TRANS_ITER < NEW_PIDS; TRANS_ITER++){
  	thr_id = pthread_create(&p_thread[TRANS_ITER], NULL, cont_counter, new_pl[TRANS_ITER]);
  	if(thr_id < 0) perror("thread create error\n");
      }
    }
//  	printf("after Process >> CUR_PID_CNT : %d\n", CUR_PID_CNT);
  	//usleep(10000);
	usleep(333333);
	//sleep(3);
  	
    if(INIT_PID_CNT > CUR_PID_CNT) INIT_PID_CNT = get_pid_cnt(ini_pl);
  }
}
int getCmdLine(char *file, char *buf) {
    FILE *srcFp;
    int i;
    srcFp = fopen(file, "r");
    memset(buf, 0, sizeof(buf));
    fgets(buf, 256, srcFp);
    fclose(srcFp);
}

  	/*char cmdLine[256];
  char tempPath[256];
  printf("[%d] ", pid);
  getCmdLine(tempPath, cmdLine); 
  printf("[%d] %s\n", pid, cmdLine);*/
