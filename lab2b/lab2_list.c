//NAME: Milad Nourian
//ID: 004854226
//EMAIL: miladnourian@ucla.edu
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "SortedList.h"
#include <signal.h>

//global data
int numThreads = 1;
int numIterations = 1;
int spinLockFlag = 0;
int mutexFlag = 0;
int listFlag = 0;
int numList = 1;
int opt_yield = 0;
int yieldFlag = 0;
int * spin_LOCK_ARRAY = NULL;
pthread_mutex_t * mutex_LOCK_ARRAY = NULL;
SortedList_t * list_head;
SortedListElement_t * elements;
SortedListElement_t ** arrayOfHeads = NULL; //global variable
long long * timeElapsedForThreads;
volatile int spinLockVar = 0;


pthread_mutex_t mutex_lock;




void handleUnrecognizedArguments (){
    fprintf(stderr, "Unrecognized argument was provided to the program, terminating with signal 1\n");
    exit(EXIT_FAILURE);
}

void produce_random_keys(int loopCount, SortedListElement_t * elements){
    srand((unsigned int)time(NULL));//seed
    int wordLength = 5;
    for (int i = 0 ; i < loopCount; i ++){
        //create 5 chars + null bytes
        char * str = malloc((wordLength + 1) * sizeof(char));
        for (int j = 0; j < wordLength; j++){
            int calc_offset = rand() %26; //since there are 26 letters in English
            str[j] = 'a' + calc_offset; //one of the alphabet letter
        }
        str[wordLength] = '\0'; //null-character so we can use str comparison later
        elements[i].key = str;
    }
    
}

void spinLock()
{
    while (__sync_lock_test_and_set(&spinLockVar, 1))
        ; //spin
}

void releaseSpinLock(){
    __sync_lock_release(&spinLockVar);
}
/*
 each thread:
 o starts with a set of pre-allocated and initialized elements (-- iterations=#)
 o inserts them all into a (single shared-by-all-threads) list
 o gets the list length
 o looks up and deletes each of the keys it had previously inserted o exits to re-join the parent thread
 */


unsigned long
hash(const char *str)
{
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}

void * threadFunc (void* arg){
    long startIndex = (long) (arg);
    //long i = numThreads * startIndex;
    long i = numIterations * startIndex;
    long intitial = i;
    for (; i< intitial + numIterations; i++){ //errors
        if (mutexFlag && !listFlag){ //lock and unlock using the mutex
            //not the beginning and ending time
            struct timespec startTime;
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            pthread_mutex_lock(&mutex_lock);
            struct timespec endTime;
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
            elapsedTime_ns += endTime.tv_nsec;
            elapsedTime_ns -= startTime.tv_nsec;
            timeElapsedForThreads[startIndex] += elapsedTime_ns;
            
            SortedList_insert(list_head, &elements[i]);
            pthread_mutex_unlock(&mutex_lock);
        }
        else if (spinLockFlag && !listFlag){
            struct timespec startTime;
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            spinLock();
            struct timespec endTime;
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
            elapsedTime_ns += endTime.tv_nsec;
            elapsedTime_ns -= startTime.tv_nsec;
            timeElapsedForThreads[startIndex] += elapsedTime_ns;
            
            SortedList_insert(list_head, &elements[i]);
            releaseSpinLock();
        }
        else if (listFlag){
            //and then check if spin or mutex is supposed to be used
            //get the element and use the hash to find out where it needs to be inserted
            const char * chosenKey = elements[i].key;
            unsigned long hashValue = hash(chosenKey)% numList;
            //hashValue is the list we have to insert into
            //so now we need to lock it
            if (spinLockFlag){
                //spinLock
                struct timespec startTime;
                clock_gettime(CLOCK_MONOTONIC, &startTime);
                while (__sync_lock_test_and_set(&spin_LOCK_ARRAY[hashValue], 1)) //acquire that specific lock for that sublist
                    ;//spin
                struct timespec endTime;
                clock_gettime(CLOCK_MONOTONIC, &endTime);
                long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
                elapsedTime_ns += endTime.tv_nsec;
                elapsedTime_ns -= startTime.tv_nsec;
                timeElapsedForThreads[startIndex] += elapsedTime_ns;
                SortedList_insert(arrayOfHeads[hashValue], &elements[i]);
                __sync_lock_release(&spin_LOCK_ARRAY[hashValue]);
            }
            else if (mutexFlag){
                struct timespec startTime;
                clock_gettime(CLOCK_MONOTONIC, &startTime);
                pthread_mutex_lock(&mutex_LOCK_ARRAY[hashValue]);
                struct timespec endTime;
                clock_gettime(CLOCK_MONOTONIC, &endTime);
                long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
                elapsedTime_ns += endTime.tv_nsec;
                elapsedTime_ns -= startTime.tv_nsec;
                timeElapsedForThreads[startIndex] += elapsedTime_ns;
                SortedList_insert(arrayOfHeads[hashValue], &elements[i]);
                pthread_mutex_unlock(&mutex_LOCK_ARRAY[hashValue]);
            }
        }
        else{
            SortedList_insert(list_head, &elements[i]);
        }
    }
    //get length starts
    long length = 0;
    if (mutexFlag && !listFlag){
        struct timespec startTime;
        clock_gettime(CLOCK_MONOTONIC, &startTime);
        pthread_mutex_lock(&mutex_lock);
        struct timespec endTime;
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
        elapsedTime_ns += endTime.tv_nsec;
        elapsedTime_ns -= startTime.tv_nsec;
        timeElapsedForThreads[startIndex] += elapsedTime_ns;
        length = SortedList_length(list_head);
        pthread_mutex_unlock(&mutex_lock);
    }
    else if (spinLockFlag && !listFlag){
        struct timespec startTime;
        clock_gettime(CLOCK_MONOTONIC, &startTime);
        spinLock();
        struct timespec endTime;
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
        elapsedTime_ns += endTime.tv_nsec;
        elapsedTime_ns -= startTime.tv_nsec;
        timeElapsedForThreads[startIndex] += elapsedTime_ns;
        
        length = SortedList_length(list_head);
        releaseSpinLock();
    }
    else if (listFlag){
        //now we have to iterate through all of the sublists
        length = 0;
        for (int j = 0; j < numList; j++){
            if (spinLockFlag){
                struct timespec startTime;
                clock_gettime(CLOCK_MONOTONIC, &startTime);
                while (__sync_lock_test_and_set(&spin_LOCK_ARRAY[j], 1)) //acquire that specific lock for that sublist
                    ;//spin
                struct timespec endTime;
                clock_gettime(CLOCK_MONOTONIC, &endTime);
                long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
                elapsedTime_ns += endTime.tv_nsec;
                elapsedTime_ns -= startTime.tv_nsec;
                timeElapsedForThreads[startIndex] += elapsedTime_ns;
                length += SortedList_length(arrayOfHeads[j]);
                __sync_lock_release(&spin_LOCK_ARRAY[j]);
            }
            else if (mutexFlag){
                pthread_mutex_lock(&mutex_LOCK_ARRAY[j]); //lock the jth list
                length += SortedList_length(arrayOfHeads[j]);
                pthread_mutex_unlock(&mutex_LOCK_ARRAY[j]);
            }
        }
    }
    else{
        length =SortedList_length(list_head);
    }
    //end of getlength
    //now lookup and delete

//    i = numThreads * startIndex + length - length;
//    for (; i < numIterations + numIterations ; i++){ //fix this
    i = numIterations * startIndex + length - length;
    intitial = i;
    for (; i< intitial + numIterations; i++){ //errors
        if (mutexFlag && !listFlag){
            struct timespec startTime;
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            pthread_mutex_lock(&mutex_lock);
            struct timespec endTime;
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
            elapsedTime_ns += endTime.tv_nsec;
            elapsedTime_ns -= startTime.tv_nsec;
            timeElapsedForThreads[startIndex] += elapsedTime_ns;
            
            SortedListElement_t * ptr = SortedList_lookup(list_head, elements[i].key); //look up the value
            if (ptr == NULL){
                fprintf(stderr, "Error with lookup, exiting\n");
                exit(EXIT_FAILURE);
            }
            SortedList_delete(ptr);
            pthread_mutex_unlock(&mutex_lock);
        }
        else if (spinLockFlag && !listFlag){
            struct timespec startTime;
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            spinLock();
            struct timespec endTime;
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
            elapsedTime_ns += endTime.tv_nsec;
            elapsedTime_ns -= startTime.tv_nsec;
            timeElapsedForThreads[startIndex] += elapsedTime_ns;
            
            
            SortedListElement_t * ptr = SortedList_lookup(list_head, elements[i].key); //look up the value
            if (ptr == NULL){
                fprintf(stderr, "Error with lookup, exiting\n");
                exit(EXIT_FAILURE);
            }
            SortedList_delete(ptr);
            releaseSpinLock();
        }
        else if (listFlag){
            //first find the correct list
            
            unsigned long indexList = hash(elements[i].key)% numList;
            if (spinLockFlag){
                struct timespec startTime;
                clock_gettime(CLOCK_MONOTONIC, &startTime);
                while (__sync_lock_test_and_set(&spin_LOCK_ARRAY[indexList], 1)) //acquire that specific lock for that sublist
                    ;//spin
                struct timespec endTime;
                clock_gettime(CLOCK_MONOTONIC, &endTime);
                long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
                elapsedTime_ns += endTime.tv_nsec;
                elapsedTime_ns -= startTime.tv_nsec;
                timeElapsedForThreads[startIndex] += elapsedTime_ns;
                //SortedList_insert(arrayOfHeads[hashValue], &elements[i]);
                SortedListElement_t * ptr = SortedList_lookup(arrayOfHeads[indexList], elements[i].key);
                if (ptr == NULL){
                    fprintf(stderr, "Error with lookup, exiting\n");
                    exit(EXIT_FAILURE);
                }
                SortedList_delete(ptr);
                __sync_lock_release(&spin_LOCK_ARRAY[indexList]);
            }
            else if (mutexFlag){
                pthread_mutex_lock(&mutex_LOCK_ARRAY[indexList]);
                SortedListElement_t * ptr = SortedList_lookup(arrayOfHeads[indexList], elements[i].key);
                if (ptr == NULL){
                    fprintf(stderr, "Error with lookup, exiting\n");
                    exit(EXIT_FAILURE);
                }
                SortedList_delete(ptr);
                pthread_mutex_unlock(&mutex_LOCK_ARRAY[indexList]);
            }
        }
        else{
            SortedListElement_t * ptr = SortedList_lookup(list_head, elements[i].key); //look up the value
            if (ptr == NULL){
                fprintf(stderr, "Error with lookup, exiting\n");
                exit(EXIT_FAILURE);
            }
            SortedList_delete(ptr);
        }
        
    }
    //end of lockup and delete
    return NULL;
}

char * getYieldOpts (){
    if ((opt_yield & INSERT_YIELD) == 0 && (opt_yield & DELETE_YIELD) == 0 && (opt_yield & LOOKUP_YIELD) == 0){
        return "none";
    }
    else if ((opt_yield & INSERT_YIELD) != 0 && (opt_yield & DELETE_YIELD) == 0 && (opt_yield & LOOKUP_YIELD) == 0){
        return "i";
    }
    else if ((opt_yield & INSERT_YIELD) == 0 && (opt_yield & DELETE_YIELD) == 0 && (opt_yield & LOOKUP_YIELD) != 0){
        return "l";
    }
    else if((opt_yield & INSERT_YIELD) == 0 && (opt_yield & DELETE_YIELD) != 0 && (opt_yield & LOOKUP_YIELD) == 0){
        return "d";
    }
    else if ((opt_yield & INSERT_YIELD) != 0 && (opt_yield & DELETE_YIELD) != 0 && (opt_yield & LOOKUP_YIELD) == 0){
        return "id";
    }
    else if((opt_yield & INSERT_YIELD) != 0 && (opt_yield & DELETE_YIELD) == 0 && (opt_yield & LOOKUP_YIELD) != 0){
        return "il";
    }
    else if ((opt_yield & INSERT_YIELD) == 0 && (opt_yield & DELETE_YIELD) != 0 && (opt_yield & LOOKUP_YIELD) != 0){
        return "dl";
    }
    else if ((opt_yield & INSERT_YIELD) != 0 && (opt_yield & DELETE_YIELD) != 0 && (opt_yield & LOOKUP_YIELD) != 0){
        return "idl";
    }
    return NULL; //error
    
}

char * getSyncOpts(){
    if (spinLockFlag){
        return "s";
    }
    else if (mutexFlag){
        return"m";
    }
    else {
        return "none";
    }
    return NULL; //on error
}



void segmentationFaultHandler(int sigNum){
    fprintf(stderr, "Segmentation fault detected,opt_yield=%d, numThreads = %d, numIterations = %d, signum = %d\n", opt_yield, numThreads, numIterations,sigNum);
    exit(2);
}


int main(int argc, char ** argv) {
    
    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", required_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {"lists", required_argument, 0, 'l'},
        {0,         0,                 0,  0 }
    };
    int c;
    
    signal(SIGSEGV, segmentationFaultHandler);
    
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
                else{
                    fprintf(stderr, "Unexpected value given to sync, exiting with failure\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                listFlag = 1;
                numList = atoi(optarg);
                break;
            case 'y':
                //iterate through all the characters and find out which ones are chosen
                for (unsigned int i = 0; i < strlen(optarg); i++){//i for insert, d for delete, l for lookups
                    if (optarg [i] == 'i'){
                        opt_yield = opt_yield | INSERT_YIELD;
                    }
                    else if (optarg[i] == 'd'){
                        opt_yield = opt_yield | DELETE_YIELD;
                    }
                    else if (optarg [i] == 'l') {
                        opt_yield = opt_yield | LOOKUP_YIELD;
                    }
                    else{
                        handleUnrecognizedArguments();
                    }
                }
                break;
            case '?':
            default:
                //this means that the user input unrecognized argument to the program
                handleUnrecognizedArguments();
                fprintf(stderr, "This should not print after the unrecognized input from the user\n");
                break;
        }
    }
    //now we have settup the yield option for different functions insert, lookup and delete
    //we have just one list and have one head
    
    if (mutexFlag){
        pthread_mutex_init(&mutex_lock, NULL);
    }
    //run --iterations=2 --threads=1 --yield=idl --sync=m
    list_head = (SortedList_t *)malloc(sizeof(SortedList_t));
    list_head->key = NULL; //to show the head
    list_head -> next = list_head;
    list_head -> prev = list_head;
    
    
    // now we have to create an array of the heads
    if (listFlag){
        //have to malloc each of the array elements also
        arrayOfHeads = (SortedListElement_t **) malloc(numList * sizeof (SortedListElement_t *));
        for (int k = 0; k < numList; k++){
            arrayOfHeads[k] = (SortedList_t *) malloc(sizeof(SortedList_t));
            //initialize each of the head pointers
            arrayOfHeads[k]->key = NULL;
            arrayOfHeads[k] -> next = arrayOfHeads[k];
            arrayOfHeads[k] -> prev = arrayOfHeads[k];
        }
        //now for each of the mutex and spin cases, use a seperate lock
        if (spinLockFlag){
            spin_LOCK_ARRAY = (int *) malloc((numList * sizeof (int)));
            memset(spin_LOCK_ARRAY, 0, numList * sizeof(int));
        }
        if (mutexFlag){
            mutex_LOCK_ARRAY = (pthread_mutex_t *) malloc(numList * sizeof (pthread_mutex_t));
            memset(mutex_LOCK_ARRAY, 0, numList * sizeof(pthread_mutex_t));
        }
    }
    
    
    //now create and initialize the list of elements
    elements = (SortedListElement_t*) malloc( (numIterations * numThreads)* sizeof(SortedListElement_t)); //elements is global
    
    int numElements = numThreads * numIterations;
    produce_random_keys(numElements, elements);
    //get the starting time
    
    //malloc an array and use memset to set it to zero and find the totaltimeelapsed
    //timeElapsedForThreads
    if (mutexFlag| spinLockFlag){ //just do this if you got a flag
        timeElapsedForThreads = (long long *) malloc(numThreads * sizeof(long));
        memset(timeElapsedForThreads, 0, numThreads * sizeof(long));
    }
    
    struct timespec startTime;
    clock_gettime(CLOCK_MONOTONIC, &startTime);
    //create the threads
    pthread_t threads [numThreads];
    for (long i = 0; i < numThreads; i ++){
        int rc =pthread_create(&threads[i], NULL, threadFunc, (void *)i); //just pass in the i value and make the elements global
        if (rc){
            fprintf(stderr, "there was an error creating the thread, exiting\n");
            exit(2);
        }
    }
    for (int i = 0; i < numThreads; i ++){
        //wait for all threads, join
        int rc = pthread_join(threads[i], NULL);
        if (rc != 0){ fprintf(stderr, "There was an error joining, exiting.\n");
            fprintf(stderr, "The error was %s \n", strerror(errno));
            exit(2);
        }
    }
    
    //now we have the numThreads and numIterations
    struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);
    long long elapsedTime_ns =  (endTime.tv_sec - startTime.tv_sec) * 1000000000; //in nanoseconds
    elapsedTime_ns += endTime.tv_nsec;
    elapsedTime_ns -= startTime.tv_nsec;
    
    //get the total acquisition time
    
    long long totalAcquisitionTime = 0;
    if (mutexFlag | spinLockFlag){
        for (int j = 0; j < numThreads; j++){
            totalAcquisitionTime += timeElapsedForThreads[j];
        }
    }
    
    long long numLockOperations = (2*numIterations + 1) * (numThreads);
    
    //now display as the 8th line
    
    if (SortedList_length(list_head) != 0){
        fprintf(stderr,"There has been corruption in the size of the list; size != 0\n");
        exit(2);
    }
    //list-yieldopts-syncopts: where
    char str [60];
    memset(str, 0, 60*sizeof(str[0])); //zero-out the entire string
    char * yieldOpts = getYieldOpts();
    char * syncopts = getSyncOpts();
    
    
    
    /*
     the name of the test, which is of the form: list-yieldopts-syncopts: where § yieldopts = {none, i,d,l,id,il,dl,idl}
     § syncopts = {none,s,m}
     o the number of threads (from --threads=)
     o the number of iterations (from --iterations=)
     o the number of lists (always 1 in this project)
     o the total number of operations performed: threads x iterations x 3 (insert
     + lookup + delete)
     o the total run time (in nanoseconds) for all threads
     o the average time per operation (in nanoseconds).
     */
    int numOperations = numThreads * numIterations * 3;
    long long averageTimePerOpt = elapsedTime_ns /numOperations;
    if (! (mutexFlag | spinLockFlag)){
    sprintf(str, "list-%s-%s,%d,%d,%d,%d,%lld,%lld,0",yieldOpts, syncopts, numThreads, numIterations, numList, numOperations,elapsedTime_ns, averageTimePerOpt);
    }
    else{
        long long timePerClockOperation = (totalAcquisitionTime/numLockOperations);
            sprintf(str, "list-%s-%s,%d,%d,%d,%d,%lld,%lld,%lld",yieldOpts, syncopts, numThreads, numIterations, numList,numOperations,elapsedTime_ns, averageTimePerOpt, timePerClockOperation);
    }
    
    printf("%s\n",str);
    
    
    if (mutexFlag){
        pthread_mutex_destroy(&mutex_lock);
    }
    //important: free also all of the characters strings
    //free the keys
    for (int i = 0; i < numThreads * numIterations; i++){
        free((void *)elements[i].key);
    }
    if (listFlag){
        free (arrayOfHeads);
        if (spinLockFlag){
            free (spin_LOCK_ARRAY );
        }
        if (mutexFlag){
            free (mutex_LOCK_ARRAY);
        }
    }
    free(elements);
    free(list_head);
    if (mutexFlag| spinLockFlag){
        free (timeElapsedForThreads);
    }
    //IMPORTANT: dont forget to free up the list_head Pointer
    //free (list_head);
    //free the elements in the elements_head and also elements_head itself
    return 0;
    
}

