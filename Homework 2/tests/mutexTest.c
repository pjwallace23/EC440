#include <pthread.h>
#include <stdlib.h>
#include<stdio.h>
#include<unistd.h>		//for sleep testing

#define THREAD_CNT 6
#define COUNTER_FACTOR 0xFFFFFF

pthread_mutex_t mutex;
pthread_t threads[THREAD_CNT];
int criticalData;

void wasteTime(int a){
	for(long int i = 0; i < a*COUNTER_FACTOR; i++);
}
void* notMutex(void *arg);
 void* mutexTest(void *arg) {
	 
   // printf("before lock\n");
	pthread_mutex_lock(&mutex);
  //  printf("after lock\n");
	criticalData++;
	printf("Thread %i start -- criticalData = %i\n",(int)pthread_self(),criticalData);
	
	wasteTime(20);
	
	printf("Thread %i finish -- criticalData = %i\n",(int)pthread_self(),criticalData);
	pthread_mutex_unlock(&mutex);
    printf("unlocked mutex\n");
	return NULL;
 }
 
 void* notMutex(void *arg){
	printf("thread %lx start -- not part of mutex\n",pthread_self());
		
	wasteTime(10);
		
	printf("thread %lx finish -- not part of mutex\n",pthread_self());
	return NULL;
 }

int main(int argc, char **argv) {
	pthread_mutex_init(&mutex, NULL);

	criticalData = 0;
	for (int i = 0; i < THREAD_CNT; i++) {
        printf("before create\n");
		pthread_create(&threads[i], NULL, &mutexTest, (void *)(intptr_t)i);
		pthread_create(&threads[i], NULL, &notMutex, (void *)(intptr_t)i);
	}
	printf("hello there\n");
	for(long int i = 0; i < THREAD_CNT; i++){
		wasteTime(20);
	}
    printf("hi\n");
	pthread_mutex_destroy(&mutex);
	return 0;
}