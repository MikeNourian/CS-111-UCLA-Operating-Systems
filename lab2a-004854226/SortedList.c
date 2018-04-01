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


/**
 * SortedList_lookup ... search sorted list for a key
 *
 *    The specified list will be searched for an
 *    element with the specified key.
 *
 *
 * @return pointer to matching element, or NULL if none is found
 */
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

/**
 * SortedList_length ... count elements in a sorted list
 *    While enumeratign list, it checks all prev/next pointers
 *
 * aram SortedList_t *list ... header for the list
 *
 * return int number of elements in list (excluding head)
 *       -1 if the list is corrupted
 */
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
