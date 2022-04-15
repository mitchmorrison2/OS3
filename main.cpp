#include "Graph.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>

int NUM_THREADS, NUM_RESOURCES;
int MAX_LINE_LEN = 25, MAX_RESOURCE_REQ = 50;
char** players;
int* num_resources_req;
int** resources_req;
int* finishedList;

pthread_mutex_t lockRequest;
pthread_mutex_t lockRelease;

bool isEmpty = 1;
Graph rag(isEmpty);


bool request(char* player, int dest, int src) {
    pthread_mutex_lock(&lockRequest);
    rag.addEdge(src, dest);
    bool cycle = rag.isCyclic();
    bool removed = rag.removeEdge(src, dest);
    pthread_mutex_unlock(&lockRequest);
    if (cycle) {
        printf("Cycle detected. Reject\n");
        return false;
    } else {
        printf("No cycle, approve request\n");
        return true;
    }
}

bool release(char* player, int src, int dest) {
    pthread_mutex_lock(&lockRelease);
    bool removed = rag.removeEdge(src, dest);
    pthread_mutex_unlock(&lockRelease);
    return removed;
}

void* runner(void* v) {
    int index = *(int*)v;
    printf("Thread %d\n", index);

    while(!finishedList[index]) {
        int i = 0;
        while (i < num_resources_req[index]) {
            int r = rand() % 100;
            long ns = r * pow(10, 7);
            if (resources_req[index][i] > 0) {
                // request resource
                bool granted = request(players[index], index, resources_req[index][i]);
                if (granted) {
                    rag.addEdge(index, resources_req[index][i]);
                    // increment counter to request next resource in list on next iteration
                    i++;

                    struct timespec remaining, request = {1, ns};
                    nanosleep(&request, &remaining);
                    printf("Waiting 1 + q/100 seconds until next request");
                }
                else {
                    struct timespec remaining, request = {0, ns};
                    nanosleep(&request, &remaining);

//                    print("Waiting q/100 seconds until next request");
                }
            }
            else {
                // release resource
                bool removed = release(players[index], index, resources_req[index][i]);
//                bool removed = rag.removeEdge(index, resources_req[index][i]);
                if (removed) {
                    printf("Successfully removed from adjList");
                }
                else {
                    printf("Item DNE and was not removed");
                }
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
    pthread_mutex_init(&lockRelease, NULL);

    rag.setList(NUM_RESOURCES);
    
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

    pthread_t tid[NUM_THREADS]; //make this an array of size numThreads
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    for (int i = 0; i < NUM_THREADS; i++) {
        int* x = (int*) malloc(sizeof(int));
        *x = i;
        pthread_create(&(tid[i]), &attr, runner, (void*)x);
    }


    // join threads at end



    // Create a graph given in the above diagram
//    Graph g(4);
//    rag.addEdge(0, 1);
//    rag.addEdge(0, 2);
//    rag.addEdge(1, 2);
//    g.addEdge(2, 3);
//    g.addEdge(2, 3);
//    g.addEdge(3, 3);

//    if(rag.isCyclic())
//        cout << "Graph contains cycle";
//    else
//        cout << "Graph doesn't contain cycle";

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
