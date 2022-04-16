#include "Graph.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <list>

int NUM_THREADS, NUM_RESOURCES;
int MAX_LINE_LEN = 25, MAX_RESOURCE_REQ = 50;
char** players;
int* num_resources_req;
int** resources_req;
int* finishedList;

pthread_mutex_t lockRequest;
pthread_mutex_t printLock;
pthread_mutex_t randProtect;

bool isEmpty = 1;
Graph* rag;


bool request(char* player, int thread, int resource) {
    pthread_mutex_lock(&lockRequest);
    // check if resource vertex already exists
    printf("Person %s requests %d\n", player, resource);
    bool vertexExists = rag->vertexExists(resource);
    if (vertexExists) {
        // if exists then only add edge to process vertex if no cycle would form
        rag->addClaimEdge(thread, resource);
        bool cycle = rag->isCyclic();
        if (cycle) {
            bool removed = rag->removeEdge(thread, resource);
            printf("Person %s requests %d: denied\n", player, resource);
        }
        else {
            printf("Person %s requests %d: accepted\n", player, resource);
        }
    }
    else {
        // if resource vertex DNE and no cycle occurs than add resource vertex edge
        rag->addEdge(resource, thread);
    }

    pthread_mutex_lock(&printLock);
//    printf("Person %s requests %d\n", player, resource);

//    if (cycle) {
//        bool removed = rag->removeEdge(resource, thread);
//        printf("Person %s requests %d: denied\n", player, resource);
//    }
//    else {
//        printf("Person %s requests %d: accepted\n", player, resource);
//    }
    pthread_mutex_unlock(&printLock);
    pthread_mutex_unlock(&lockRequest);

    return !cycle;
}

bool release(char* player, int requester, int resource) {
    pthread_mutex_lock(&lockRequest);
    pthread_mutex_lock(&printLock);

//    bool removed = rag->removeEdge(resource, requester);
    if (resource < 0) {
        resource *= -1;
    }
    rag->releaseResourceVertex(resource, requester);

    printf("Person %s releases %d\n", player, resource);
    pthread_mutex_unlock(&printLock);
    pthread_mutex_unlock(&lockRequest);
    return 0;
}

void* runner(void* v) {
    int index = *(int*)v;
    printf("Thread %d\n", index);

    while(!finishedList[index]) {
        int i = 0;
        while (i < num_resources_req[index]) {
            pthread_mutex_lock(&randProtect);
            long r = rand() % 100;
            pthread_mutex_unlock(&randProtect);

            long ns = r * pow(10, 7); // 10**9 / 100 == 10**7
            if (resources_req[index][i] >= 0) {
                // request resource
                bool granted = request(players[index], index, resources_req[index][i]);
                if (granted) {
                    // increment counter to request next resource in list on next iteration
                    i++;
                    struct timespec remaining, request = {1, ns};
                    nanosleep(&request, &remaining);
                }
                else {
                    struct timespec remaining, request = {0, ns};
                    nanosleep(&request, &remaining);
                }
            }
            else {
                // release resource
                bool removed = release(players[index], index, resources_req[index][i]);
                if (removed) {
                    printf("Successfully removed from adjList\n");
                }
                else {
                    printf("Item DNE and was not removed");
                }
                i++;
            }

        }
        pthread_mutex_lock(&randProtect);
        long r = rand() % 100;
        pthread_mutex_unlock(&randProtect);
        long ns = r * pow(10, 7); // 10**9 / 100 == 10**7
        struct timespec remaining, request = {1, ns};
        nanosleep(&request, &remaining);

        for (int c = 0; c < NUM_RESOURCES; c++) {
            if (resources_req[index][c] > 0 && rag->edgeExists(index, resources_req[index][c])) {
                // release all resources that thread INDEX is using
                release(players[index], index, resources_req[index][c]);
            }
        }

        finishedList[index] = 1;
    }

    pthread_exit(0);

}

int main(int argc, char** argv) {
    FILE* fp;
    fp = fopen(argv[1], "r");
    
    if(!fp){
        printf("Can't open file\n");
        exit(1);
    }

    fscanf(fp, "%d %d\n", &NUM_THREADS, &NUM_RESOURCES);
    // allocate space for each global variable
    players = (char**)malloc(sizeof(char*)*NUM_THREADS);
    num_resources_req = (int*)malloc(sizeof(int)*NUM_THREADS);
    resources_req = (int**)malloc(sizeof(int*)*NUM_THREADS);
    finishedList = (int*)malloc(sizeof(int)*NUM_THREADS);

    pthread_mutex_init(&lockRequest, NULL);
    pthread_mutex_init(&randProtect, NULL);
    pthread_mutex_init(&printLock, NULL);

//    rag.setList(NUM_RESOURCES);
    rag = new Graph(NUM_RESOURCES);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        //create global storage for players names and resource requests
        players[i] = (char*)malloc(sizeof(char)*MAX_LINE_LEN);
        resources_req[i] = (int*)malloc(sizeof(int)*MAX_RESOURCE_REQ);
        finishedList[i] = 0;
        // read in each line to respective variables
        fscanf(fp, "%s %d ", players[i], &num_resources_req[i]);
        printf("%s %d: ", players[i], num_resources_req[i]);
        for (int c = 0; c < num_resources_req[i]; c++) {
            fscanf(fp, "%d ", &resources_req[i][c]);
            printf("%d ", resources_req[i][c]);
        }
        fscanf(fp, "\n");
        printf("\n");
    }

    pthread_t tid[NUM_THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    for (int i = 0; i < NUM_THREADS; i++) {
        int* x = (int*) malloc(sizeof(int));
        *x = i;
        pthread_create(&(tid[i]), &attr, runner, (void*)x);
    }


    // join threads at end

    while(true) {
        bool allTrue = true;
        for (int i = 0; i < NUM_THREADS; i++) {
            if (finishedList[i] == 0) {
                allTrue = false;
            }
        }
        if (allTrue) {
            break;
        }
    }
    printf("All threads are finished and have exited");

    return 0;
}
