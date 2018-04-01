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
int opt_yield = 0;
int yieldFlag = 0;
SortedList_t * list_head;
SortedListElement_t * elements;

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

void * threadFunc (void* arg){
    long startIndex = (long) (arg);
    long i = numThreads * startIndex;
    for (; i< numIterations; i++){
        if (mutexFlag){ //lock and unlock using the mutex
            pthread_mutex_lock(&mutex_lock);
            SortedList_insert(list_head, &elements[i]);
            pthread_mutex_unlock(&mutex_lock);
        }
        else if (spinLockFlag){
            spinLock();
            SortedList_insert(list_head, &elements[i]);
            releaseSpinLock();
        }
        else{
            SortedList_insert(list_head, &elements[i]);
        }
    }
    long length;
    if (mutexFlag){
        pthread_mutex_lock(&mutex_lock);
        length = SortedList_length(list_head);
        pthread_mutex_unlock(&mutex_lock);
    }
    else if (spinLockFlag){
        spinLock();
        length = SortedList_length(list_head);
        releaseSpinLock();
    }
    else{
        length =SortedList_length(list_head);
    }
    
    //now lookup and delete
    i = numThreads * startIndex + length - length;
    for (; i < numIterations; i++){
        if (mutexFlag){
            pthread_mutex_lock(&mutex_lock);
            SortedListElement_t * ptr = SortedList_lookup(list_head, elements[i].key); //look up the value
            if (ptr == NULL){
                fprintf(stderr, "Error with lookup, exiting\n");
                exit(EXIT_FAILURE);
            }
            SortedList_delete(ptr);
            pthread_mutex_unlock(&mutex_lock);
        }
        else if (spinLockFlag){
            spinLock();
            SortedListElement_t * ptr = SortedList_lookup(list_head, elements[i].key); //look up the value
            if (ptr == NULL){
                fprintf(stderr, "Error with lookup, exiting\n");
                exit(EXIT_FAILURE);
            }
            SortedList_delete(ptr);
            releaseSpinLock();
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
    return NULL;
}

/*
 
 void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
 //reading the (list) is a critical section since that means we have to exclusive access to the list and not anyone else writing to it
 //first call malloc
 if (opt_yield & INSERT_YIELD){ //before the critical section
 sched_yield();
 }
 SortedListElement_t * toBeInsertedElements = (SortedListElement_t *) malloc(sizeof(SortedListElement_t));
 if (toBeInsertedElements == NULL){fprintf(stderr, "There was an error mallocing\n");
 exit(EXIT_FAILURE);
 }
 toBeInsertedElements->key = element -> key;
 SortedListElement_t * curr = list;
 //head has data to be null
 //curr -> key < toBeInserted -> key
 //if empty list
 if ((curr -> next)->key == NULL){
 list -> next = toBeInsertedElements;
 list -> prev = toBeInsertedElements;
 toBeInsertedElements -> next = list;
 toBeInsertedElements -> prev = list;
 return;
 }
 const char * str1 =(curr -> next) -> key;
 const char * str2 = toBeInsertedElements ->key;
 while ((curr-> next)->key != NULL && (strcmp(str1, str2) <= 0)){
 curr = curr -> next;
 }
 //now curr has to put to the element on it next
 SortedListElement_t * nextElement = curr -> next;
 curr -> next = toBeInsertedElements;
 nextElement -> prev = toBeInsertedElements;
 toBeInsertedElements -> prev = curr;
 toBeInsertedElements -> next = nextElement;
 }
 */

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *    The specified element will be removed from whatever
 *    list it is currently in.
 *
 *    Before doing the deletion, we check to make sure that
 *    next->prev and prev->next both point to this node
 *
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */

/*
 int SortedList_delete( SortedListElement_t *element){
 if (opt_yield & DELETE_YIELD){ //before the critical section
 sched_yield();
 }
 if (element ->key == NULL){
 fprintf(stderr, "Should not delete the head in Sorted_delete\n");
 exit(2);
 }
 if ((element -> next)->prev != element || (element -> prev)->next != element){
 return 1;
 }
 //now changing the pointer
 (element -> next)->prev = element -> prev;
 (element -> prev) -> next = element -> next;
 free (element); //this way allocated
 return 0;
 }
 */

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *    The specified list will be searched for an
 *    element with the specified key.
 *
 *
 * @return pointer to matching element, or NULL if none is found
 */
/*
 SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
 if (opt_yield & LOOKUP_YIELD){ //before the critical section
 sched_yield();
 }
 SortedListElement_t * curr = list -> next;
 //lookup
 while (curr -> key != NULL){
 if (strcmp(curr -> key, key) == 0){
 //you have found the value
 return curr;
 }
 curr = curr -> next;
 }
 //could not find it
 return NULL;
 }
 */

/**
 * SortedList_length ... count elements in a sorted list
 *    While enumeratign list, it checks all prev/next pointers
 *
 * aram SortedList_t *list ... header for the list
 *
 * return int number of elements in list (excluding head)
 *       -1 if the list is corrupted
 */
/*
 int SortedList_length(SortedList_t *list){
 if (opt_yield & LOOKUP_YIELD){ //before the critical section
 sched_yield();
 }
 int length = 0;
 SortedListElement_t * curr = list -> next; //get the second element
 while (curr -> key != NULL) {
 length ++;
 curr = curr -> next;
 }
 return length;
 }
 */
/*
 form: list-yieldopts-syncopts:
 yieldopts = {none, i,d,l,id,il,dl,idl}
 syncopts = {none,s,m}
 */

//int opt_yield = 0;
//#define    INSERT_YIELD    0x01    // yield in insert critical section
//#define    DELETE_YIELD    0x02    // yield in delete critical section
//#define    LOOKUP_YIELD    0x04    // yield in lookup/length critical esction

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
    
    list_head = (SortedList_t *)malloc(sizeof(SortedList_t));
    list_head->key = NULL; //to show the head
    list_head -> next = list_head;
    list_head -> prev = list_head;
    
    //now create and initialize the list of elements
    elements = (SortedListElement_t*) malloc( (numIterations * numThreads)* sizeof(SortedListElement_t)); //elements is global
    
    int numElements = numThreads * numIterations;
    produce_random_keys(numElements, elements);
    //get the starting time
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
    sprintf(str, "list-%s-%s,%d,%d,1,%d,%lld,%lld",yieldOpts, syncopts, numThreads, numIterations,numOperations,elapsedTime_ns, averageTimePerOpt);
    
    printf("%s\n",str);
    
    
    if (mutexFlag){
        pthread_mutex_destroy(&mutex_lock);
    }
    //important: free also all of the characters strings
    //free the keys
    for (int i = 0; i < numThreads * numIterations; i++){
        free((void *)elements[i].key);
    }
    free(elements);
    free(list_head);
    
    //IMPORTANT: dont forget to free up the list_head Pointer
    //free (list_head);
    //free the elements in the elements_head and also elements_head itself
    return 0;
    
}

