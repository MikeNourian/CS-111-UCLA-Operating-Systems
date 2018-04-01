//
//  main.c
//  lab_2add
//
//  Created by Mike Nourian on 2/13/18.
//  Copyright © 2018 Mike Nourian. All rights reserved.
//

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>




void handleUnrecognizedArguments (){
    fprintf(stderr, "Unrecognized argument was provided to the program, terminating with signal 1\n");
    exit(EXIT_FAILURE);
}
//
struct Accumulate{
    long long * counterPtr;
    int numIterations;
};


int opt_yield = 0; //global var that can be used for controlling yelding
int numThreads = 1;
int numIterations = 1;


int yieldFlag = 0;
int mutexFlag=0;
int spinLockFlag = 0;
int compareAndSwapFlag = 0;

volatile int spinLockVar = 0;
volatile int compareAndSwapLockVar = 0;

void spinLock()
{
    while (__sync_lock_test_and_set(&spinLockVar, 1))
        while (spinLockVar)
            ;
}

void releaseSpinLock(){
    __sync_lock_release(&spinLockVar);
}

void compAndSwapLock(){
    while (__sync_bool_compare_and_swap(&compareAndSwapLockVar, 0, 1)) //while the lock is held
        ;
}

void compAndSwapUnlock(){
    compareAndSwapLockVar = 0; //so that the lock is free now
}

pthread_mutex_t lock; //globally shared
void performAdd(long long *pointer, long long value)
{
    long long sum = *pointer + value;
    if(opt_yield) { sched_yield(); }
    *pointer = sum;
}

void add(long long *pointer, long long value) //pointer is a pointer to counter
{
    //now change add for doing synch
    if (mutexFlag){
        pthread_mutex_lock(&lock);
        performAdd(pointer, value);
        pthread_mutex_unlock(&lock);
    }
    else if (spinLockFlag){
        //type __sync_lock_test_and_set (type *ptr, type value, ...)
        spinLock();
        performAdd(pointer, value);
        releaseSpinLock();
    }
    //the yield check should be put between the computation of the new sum and performing the compare-and-swap.
    else if (compareAndSwapFlag){
        //can also use the pointer
       // long long sum = *pointer + value;
        long long addedVal, expectedVal;
        do {
            expectedVal = * pointer;
            addedVal = expectedVal + value;
            if (opt_yield){
                sched_yield();
            }
        }while(expectedVal != __sync_val_compare_and_swap(pointer, expectedVal, addedVal));
    }
    else{
        performAdd(pointer, value); //if no synchronization specified, do the peformAdd without protection
    }
}
void * addWrapper(void* arg){
    //depending on the flag specified by the user, change the execution
    long long * counterPtr = (long long *) arg;
    //now run the function
    for (int i = 0; i < numIterations; i++){
        add(counterPtr, 1);
    }
    for (int i = 0; i < numIterations; i++){
        add(counterPtr, -1);
    }
    return NULL;
}


int main(int argc, char ** argv) {
    
    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", no_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {0,         0,                 0,  0 }
    };
    int c;
    
    long long counter = 0;
    while (1) {
        c = getopt_long(argc, argv, "", long_options, 0);
        if (c == -1){
            //if no input is given
            break;
        }
        switch (c) {
            case 't':
                numThreads = atoi(optarg);
                break;
            case 'i':
                numIterations = atoi(optarg);
                break;
            case 's':
                if (optarg[0] == 'm'){//pthread mutex is to be used
                    mutexFlag = 1;
                }
                else if (optarg[0] == 's')
                    spinLockFlag = 1;
                else if (optarg[0] == 'c')
                    compareAndSwapFlag = 1;
//                char op = optarg[0]; //get the character
                break;
            case 'y':
                opt_yield = 1;
                yieldFlag = 1;
                break;
            case '?':
            default:
                //this means that the user input unrecognized argument to the program
                handleUnrecognizedArguments();
                fprintf(stderr, "This should not print after the unrecognized input from the user\n");
                break;
        }
    }
    pthread_t threads[numThreads];
    if (mutexFlag){
        if (pthread_mutex_init(&lock, NULL) != 0)
        {
            fprintf(stderr, "Initializing the mutex failed, exiting\n");
            exit(EXIT_FAILURE);
        }
    }
    struct timespec startTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);
//    struct Accumulate accum;
//    accum.counterPtr = &counter;
//    accum.numIterations = numIterations;
    for (int i = 0; i < numThreads; i++){
        //int rc = pthread_create(&threads[i], NULL, add, (void *) &accum);
        int rc = pthread_create(&threads[i], NULL, addWrapper, &counter);
        if (rc){
            fprintf(stderr, "there was an error creating the thread, exiting\n");
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < numThreads; i ++){
        //wait for all threads, join
        int rc = pthread_join(threads[i], NULL);
        if (rc != 0){ fprintf(stderr, "There was an error joining, exiting.\n");
            fprintf(stderr, "The error was %s \n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    //now we have the numThreads and numIterations
    struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
    elapsedTime_ns += endTime.tv_nsec;
    elapsedTime_ns -= startTime.tv_nsec;
    //figure the CSV
    //create a big char * array and zero them out and then use, snprintf
    char str [60];
    memset(str, 0, 60*sizeof(str[0])); //zero-out the entire string
    //add-none,10,10000,200000,6574000,32,374
    int numOperations = numThreads * numIterations * 2;
    long long averageTimePerOpt = elapsedTime_ns /numOperations;
    
    /////CHECK: The tags need to be changed for each of the different flags accordingly
    
    /*
     • add-m ... no yield, mutex synchronization
     • add-s ... no yield, spin-lock synchronization
     • add-c ... no yield, compare-and-swap synchronization
     • add-yield-none ... yield, no synchronization
     • add-yield-m ... yield, mutex synchronization
     • add-yield-s ... yield, spin-lock synchronization
     • add-yield-c ... yield, compare-and-swap synchronization
     */
    if (!yieldFlag && mutexFlag){
        sprintf(str, "add-m,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt, counter);
    }
    else if (!yieldFlag && spinLockFlag){
        sprintf(str, "add-s,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt, counter);
    }
    else if (!yieldFlag && compareAndSwapFlag){
        sprintf(str, "add-c,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt, counter);
    }
    else if (yieldFlag && mutexFlag) {
        sprintf(str, "add-yield-m,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt, counter);
    }
    else if (yieldFlag && spinLockFlag){
        sprintf(str, "add-yield-s,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt, counter);
    }
    else if (yieldFlag && compareAndSwapFlag){
        sprintf(str, "add-yield-c,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt, counter);
    }
    else if (yieldFlag){
        sprintf(str, "add-yield-none,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt, counter);
    }
    else{//nothing specicified
        sprintf(str, "add-none,%d,%d,%d,%lld,%lld,%lld",numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt, counter);
    }
    printf("%s\n",str);
    
    if (mutexFlag){
        pthread_mutex_destroy(&lock);
    }
    
    return 0;
}




/*



while (1) {
    c = getopt_long(argc, argv, "", long_options, 0);
    if (c == -1){
        //if no input is given
        break;
    }
    switch (c) {
        case 's'://set the shellflag
            //shellFlag = 1;
            break;
        case 'p':
            portFlag = 1;
            portNumber = atoi(optarg); //to get the port number
            break;
        case 'l':
            logFlag = 1;
            logFileName = optarg;
            break;
        case 'c':
            compressFlag = 1;
            break;
        case '?':
        default:
            //this means that the user input unrecognized argument to the program
            handleUnrecognizedArguments();
            fprintf(stderr, "This should not print after the unrecognized input from the user\n");
            break;
    }
}
 */

