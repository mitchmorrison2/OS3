#include "Graph.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <list>
#include <string>

int NUM_THREADS, NUM_RESOURCES;
int MAX_LINE_LEN = 25, MAX_RESOURCE_REQ = 50;
char** players;
int* num_resources_req;
int** resources_req;
int* finishedList;

pthread_mutex_t lockRequest;
pthread_mutex_t printLock;
pthread_mutex_t randProtect;

Graph* rag;

bool request(char* player, int thread, int resource) {
    pthread_mutex_lock(&lockRequest);
    pthread_mutex_lock(&printLock);
    // check if resource vertex already exists
    printf("Person %s requests %d\n", player, resource);
    string person = string(player);
    string resourceStr = to_string(resource);

    bool c = false;
    if (rag->vertexExists(resourceStr)) {
        // if exists then only add edge to process vertex if no cycle would form
        rag->addClaimEdge(person, resourceStr);
        if (rag->isCyclic()) {
            bool removed = rag->removeEdge(person, resourceStr);
            printf("Person %s requests %d: denied\n", player, resource);
            c = false;
        }
        else {
            printf("Person %s requests %d: accepted\n", player, resource);
            c = true;
        }
    }
    else {
        // if resource vertex DNE and no cycle occurs than add resource vertex edge
        rag->addEdge(resourceStr, person);
        // see why this if statement is causing the edge <person, resourceStr> to be added when only <resourceStr, person> should be added to the graph
//        if (rag->isCyclic()) {
//            rag->removeEdge(resourceStr, person);
//        }
//        rag->removeEdge(person, resourceStr);
        c = true;
        printf("Person %s requests %d: accepted\n", player, resource);
    }
    char* p = rag->printGraph();
    printf(p);

    pthread_mutex_unlock(&printLock);
    pthread_mutex_unlock(&lockRequest);

    return c;
}

bool release(char* player, int requester, int resource) {
    pthread_mutex_lock(&lockRequest);
    pthread_mutex_lock(&printLock);

    if (resource < 0) {
        resource *= -1;
    }

    string person = string(player);
    string resourceStr = to_string(resource);

//    bool removed = rag->removeEdge(resource, player);

    size_t sz = rag->releaseResourceVertex(resourceStr, player);
    // update graph to reallocate the resource that was just freed
    if (sz) {
        // if sz > 0 then the vertex was removed
        int resourceReallocated = rag->updateGraph(resourceStr);
        if (resourceReallocated) {
            printf("Reallocated resource %d\n", resource);
        }
    }
    printf("Person %s releases %d\n", player, resource);
    char* p = rag->printGraph();
    printf(p);
    pthread_mutex_unlock(&printLock);
    pthread_mutex_unlock(&lockRequest);
    return sz;
}

void* runner(void* v) {
    int index = *(int*)v;
    printf("Thread %d - player %s\n", index, players[index]);
    char* person = players[index];
    string personS = string(person);
    list<int> resourcesToHold;

    while(!finishedList[index]) {
        int i = 0;
        while (i < num_resources_req[index]) {
            pthread_mutex_lock(&randProtect);
            long r = rand() % 100;
            pthread_mutex_unlock(&randProtect);

            long ns = r * pow(10, 7); // 10**9 / 100 == 10**7
            if (resources_req[index][i] >= 0) {
                // request resource
                resourcesToHold.push_back(resources_req[index][i]);
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
                if (rag->edgeExists(to_string(resources_req[index][i]*-1), players[index])) {
                    // only release resource if the current thread has access to it
                    resourcesToHold.remove(resources_req[index][i]*-1);
                    bool removed = release(players[index], index, resources_req[index][i]);
                    i++;
                }

            }

        }

        // check if all resource that this thread has requested are allocated to it
        while(true) {
            // if yes, release all resources and update finishedList[index] to true
            // if no, continue checking
            bool allHeld = true;
            for (auto &val: resourcesToHold) {
                if (!rag->edgeExists(to_string(val), players[index])) {
                    allHeld = false;
                }
            }
            if (allHeld) {
                printf("Thread %s finished and will next release all of its resources", players[index]);
                break;
            }
        }


        pthread_mutex_lock(&randProtect);
        long r = rand() % 100;
        pthread_mutex_unlock(&randProtect);
        long ns = r * pow(10, 7); // 10**9 / 100 == 10**7
        struct timespec remaining, request = {1, ns};
        nanosleep(&request, &remaining);

        // remove edge function not removeResourceVertex for removing the thread and its corresponsing respources
//        pthread_mutex_lock(&lockRequest);
        for (auto &v: resourcesToHold) {
            if (rag->edgeExists(to_string(v), players[index])) {
                if (release(players[index], index, v)) {
                    printf("BIG RELEASE edge %s ---> %d\n", players[index], v);
                }
            }
        }
//        pthread_mutex_unlock(&lockRequest);

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
