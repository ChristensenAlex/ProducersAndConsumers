#include "common.h"
#include "common_threads.h"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#define TRUE 1
#define FALSE 0
#define MAX 100

// Uses coarse grained locks, optimized for a single processor. Running on multipul processors won't provide speedup, until fine-grained locks are used.
pthread_mutex_t file_lock;

//Helpers for circular array
pthread_mutex_t circular_lock;	//Lock for entire circular array

//The data structure is a circular array
int buffer[100];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;

//Running total
int buckets[5] = {0};
pthread_mutex_t bucket_lock;

//condition variables
pthread_cond_t fill, empty;

// The same put function as book
// Lock before calling this function
void put(int value){
	buffer[fill_ptr] = value;
	fill_ptr = (fill_ptr + 1) % MAX; // Next spot in a circular array
	count++;
}

// Complimentry to put function
int get(){
	int value = buffer[use_ptr];
	use_ptr = (use_ptr + 1) % MAX;
	count--;
	return value;	
}

// Creates rand numbers to pipe to consumer
void producer(){
	for (int i = 0; TRUE; i++){
		int value = rand()%5;//produces a number to place on queue
		Pthread_mutex_lock(&circular_lock);
		while (count == MAX){
			Pthread_cond_wait(&empty, &circular_lock);
		}
		put(value);
		Pthread_cond_signal(&fill);// Must I signal before releasing the lock
		Pthread_mutex_unlock(&circular_lock);
	}

}

// Adds numbers to running total
void consumer(){
	while(TRUE){
		//get number from queue
		Pthread_mutex_lock(&circular_lock);
		while (count == 0){
			Pthread_cond_wait(&fill, &circular_lock);
		}
		int value = get();
		Pthread_cond_signal(&empty);
		Pthread_mutex_unlock(&circular_lock);
		
		//add to running total
		Pthread_mutex_lock(&bucket_lock);
		buckets[value]++;
		Pthread_mutex_unlock(&bucket_lock);
		printf("%d \n", value); //I print after release lock, so other processes can run.
	}
}




//This spins up the producers and consumers
//FUTURE PLAN: this will exit after total reaches a large number
int main (int argc, char *argv[]){
	FILE* output = fopen("output.txt", "w+");
	pthread_t p1, p2, p3, c1, c2;
	Pthread_create(&p1, NULL, &producer, NULL);
	Pthread_create(&p2, NULL, &producer, NULL);
	Pthread_create(&p3, NULL, &producer, NULL);
	Pthread_create(&c1, NULL, &consumer, NULL);
	Pthread_create(&c2, NULL, &consumer, NULL);

	sleep(3);
	
	pthread_kill(&p1);
	pthread_kill(&p2);
	pthread_kill(&p3);
	pthread_kill(&c1);
	pthread_kill(&c2);


	//both producer and consumer run till killed, so joins not included.
	for(int i = 0; i < 5; i++){
		printf("   %d: %ld", i, buckets[i]);
	}
				

}
