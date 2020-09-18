/*
Name: Fabiane Yeung
Instructor: Ben McCamish
CS 360 - Assignment 8 - Sort 

this file worked with the main that I created so it should work accordingly.
*/

#include "sort.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

#define SORT_THRESHOLD 40

typedef struct _sortParams
{
    char **array;
    int left;
    int right;
} SortParams;

struct strItem //my struct
{
    struct _sortParams par;
    struct strItem *next;
} *s = NULL;

static int maximumThreads = 1; /* maximum # of threads to be used */

int totalCountThread = 0;
pthread_mutex_t mutex;
pthread_cond_t waking;

void structFunc(struct _sortParams *par) //puts in order
{
    struct strItem *item = malloc(sizeof(struct strItem)); //allocates space

    //ordering it
    item->par.left = par->left;
    item->par.right = par->right;
    item->par.array = par->array;
    item->next = NULL;

    pthread_mutex_lock(&mutex); //locks

    if (s == NULL)
    {
        pthread_cond_signal(&waking); //wakes if NULL
        s = item;
    }
    else
    {
        item->next = s; //go to the next one
        s = item;
    }
}

void add(struct _sortParams *par) //sorts and them unlocks it
{
    structFunc(par);
    pthread_mutex_unlock(&mutex);
}

/* This is an implementation of insert sort, which although it is */
/* n-squared, is faster at sorting short lists than quick sort,   */
/* due to its lack of recursive procedure call overhead.          */

static void insertSort(char **array, int left, int right)
{
    int i, j;
    for (i = left + 1; i <= right; i++)
    {
        char *pivot = array[i];
        j = i - 1;
        while (j >= left && (strcmp(array[j], pivot) > 0))
        {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = pivot;
    }
}

/* Recursive quick sort, but with a provision to use */
/* insert sort when the range gets small.            */

static void *quickSort(void *par)
{
    SortParams *params = (SortParams *)par;
    char **array = params->array;
    int left = params->left;
    int right = params->right;
    int i = left, j = right;

    if (j - i > SORT_THRESHOLD) /* if the sort range is substantial, use quick sort */
    {
        int m = (i + j) >> 1; /* pick pivot as median of         */
        char *temp, *pivot;   /* first, last and middle elements */
        if (strcmp(array[i], array[m]) > 0)
        {
            temp = array[i];
            array[i] = array[m];
            array[m] = temp;
        }
        if (strcmp(array[m], array[j]) > 0)
        {
            temp = array[m];
            array[m] = array[j];
            array[j] = temp;
            if (strcmp(array[i], array[m]) > 0)
            {
                temp = array[i];
                array[i] = array[m];
                array[m] = temp;
            }
        }
        pivot = array[m];

        for (;;)
        {
            while (strcmp(array[i], pivot) < 0)
                i++; /* move i down to first element greater than or equal to pivot */
            while (strcmp(array[j], pivot) > 0)
                j--; /* move j up to first element less than or equal to pivot      */
            if (i < j)
            {
                char *temp = array[i]; /* if i and j have not passed each other */
                array[i++] = array[j]; /* swap their respective elements and    */
                array[j--] = temp;     /* advance both i and j                  */
            }
            else if (i == j)
            {
                i++;
                j--;
            }
            else
                break; /* if i > j, this partitioning is done  */
        }

        SortParams first;
        first.array = array;
        first.left = left;
        first.right = j;
        add(&first);

        SortParams second;
        second.array = array;
        second.left = i;
        second.right = right;
        quickSort(&second);
    }
    else
        insertSort(array, i, j); /* for a small range use insert sort */
}

/* user interface routine to set the number of threads sortT is permitted to use */

void setSortThreads(int count)
{
    maximumThreads = count;
}

void unlock_lock(int i) //checks for the lock
{
    if (i == 1) // lock
    {
        pthread_mutex_lock(&mutex);
    }
    else if (i == 0) // unlock
    {
        pthread_mutex_unlock(&mutex);
    }
    else if (i == 2) // waking
    {
        pthread_cond_wait(&waking, &mutex);
    }
    else if (i == 3)
    {
        pthread_cond_signal(&waking);
    }
}

void tStructFunction()
{
    SortParams params;
    struct strItem *item = s;
    s = item->next;
    params.left = item->par.left;
    params.right = item->par.right;
    params.array = item->par.array;

    free(item);
    totalCountThread++;

    unlock_lock(0); //unlock
    quickSort(&params);
    unlock_lock(1); //lock
}

void *threadFunc(void *a)
{
    int id = *(int *)a;
    free(a);

    while (1)
    {
        SortParams params;
        pthread_mutex_lock(&mutex); //lock

        if (s == NULL)
        {
            if (totalCountThread == 0)
            {
                pthread_mutex_unlock(&mutex); //unlock
                break;
            }

            pthread_cond_wait(&waking, &mutex); //wakes
        }

        if (s == NULL)
        {
            pthread_mutex_unlock(&mutex); //unlock if NULL
            continue;
        }

        pthread_mutex_unlock(&mutex); //unlock

        quickSort(&params);

        pthread_mutex_lock(&mutex); //locks again after
        tStructFunction();
        totalCountThread = totalCountThread - 1;

        if (totalCountThread == 0) //wakes
        {
            pthread_cond_signal(&waking);
        }

        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_lock(&mutex);   //lock
    pthread_cond_signal(&waking); // get the signal
    pthread_mutex_unlock(&mutex); //unlock
}

/* user callable sort procedure, sorts array of count strings, beginning at address array */

void sortThreaded(char **array, unsigned int count) //will sort them out
{
    SortParams parameters;
    parameters.array = array;
    parameters.left = 0;
    parameters.right = count - 1;

    //initializing threads
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&waking, NULL);

    add(&parameters); //calls the add function

    pthread_t *threads = malloc(sizeof(pthread_t) * maximumThreads); //allocating space

    for (int i = 0; i < maximumThreads; i++) //creating
    {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, threadFunc, (void *)id);

        if (i < 0)
        {
            fprintf(stderr, " Creation failed %s\n", strerror(errno));
            exit(1);
        }
    }

    for (int i = 0; i < maximumThreads; i++) //running
    {
        pthread_join(threads[i], NULL);

        if (i < 0)
        {
            fprintf(stderr, " join failed %s\n", strerror(errno));
            exit(1);
        }
    }

    free(threads); //frees memory when done
}