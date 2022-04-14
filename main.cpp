#include "Graph.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    FILE* fp;
    fp = fopen(argv[1], "r");
    int NUM_LINES, NUM_RESOURCES;
    int MAX_LINE_LEN = 25, MAX_RESOURCE_REQ = 50;
    char** players;
    int* num_resources_req;
    int** resources_req;


    if(!fp){
        printf("Can't open file\n");
        exit(1);
    }
    else {
        fscanf(fp, "%d %d\n", &NUM_LINES, &NUM_RESOURCES);
        players = (char**)malloc(sizeof(char*)*NUM_LINES);
        num_resources_req = (int*)malloc(sizeof(int)*NUM_LINES);
        resources_req = (int**)malloc(sizeof(int*)*NUM_LINES);
        for (int i = 0; i < NUM_LINES; i++) {
            //create global storage for players names and resource requests
            players[i] = (char*)malloc(sizeof(char)*MAX_LINE_LEN);
            resources_req[i] = (int*)malloc(sizeof(int)*MAX_RESOURCE_REQ);
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
    }


    // Create a graph given in the above diagram
    Graph g(4);
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 2);
//    g.addEdge(2, 3);
//    g.addEdge(2, 3);
//    g.addEdge(3, 3);

    if(g.isCyclic())
        cout << "Graph contains cycle";
    else
        cout << "Graph doesn't contain cycle";
    return 0;
}
