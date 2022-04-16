//
// Created by Mitchell Morrison on 4/13/22.
//

#ifndef PROGRAM3_GRAPH_H
#define PROGRAM3_GRAPH_H

// A C++ Program to detect cycle in a graph
#include <list>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <map>
#include <string>

using namespace std;

class Graph
{
    int V; // No. of vertices
//    list<int> *adj; // Pointer to an array containing adjacency lists
    bool isCyclicUtil(int v, bool visited[], bool *rs); // used by isCyclic()
    map<string, list<string>> graph;
    pthread_mutex_t lockRelease;
public:
    Graph(bool isEmpty);
    Graph(int V); // Constructor
    void addEdge(int v, int w); // to add an edge to graph
    bool removeEdge(int v, int w); // to add an edge to graph
    bool edgeExists(int idx, int resource) ;
    void addClaimEdge(int thread, int resource);
    bool vertexExists(int resource) ;
    void releaseResourceVertex(int v, int w);
    bool isCyclic(); // returns true if there is a cycle in this graph
    char* printGraph();
};

Graph::Graph(bool isEmpty) {
    printf("Created graph class\n");
}

Graph::Graph(int V)
{
    this->V = V;
//    adj = new list<int>[V];
    pthread_mutex_init(&lockRelease, NULL);

}

bool Graph::edgeExists(int idx, int resource) {
// check what resources are being used by this thread and return
// let returned function call free resource on those
//    bool found = (find(adj[resource-1].begin(), adj[resource-1].end(), idx) != adj[resource-1].end());
    bool found = (find(graph.begin(), graph.end(), resource) != graph.end());
    if (found) {
        found = (find(graph[str(resource)].begin(), graph[str(resource)].end(), str(idx)) != graph[str(resource)].end());
    }
    return found;
}

bool Graph::vertexExists(int resource) {
// check what resources are being used by this thread and return
// let returned function call free resource on those
//    bool found = (find(adj[resource-1].begin(), adj[resource-1].end(), idx) != adj[resource-1].end());
    bool found = (find(graph.begin(), graph.end(), str(resource)) != graph.end());
    return found;
}

void Graph::addEdge(int v, int w)
{
    pthread_mutex_lock(&lockRelease);
    // if not causing a cycle or if resource vertex dne then add
    // graph class add vertex
    graph[str(v)].push_back(str(w));
//    adj[v-1].push_back(w); // Add w to v’s list.
    pthread_mutex_unlock(&lockRelease);
}

void Graph::addClaimEdge(int thread, int resource) {
    // add claim edge. Key is thread and value is list of all resources it is requesting that have not yet been turned into resource edge
    graph[str(thread)].push_back(str(resource));
}


bool Graph::removeEdge(int v, int w) {
    pthread_mutex_lock(&lockRelease);
    v = abs(v);
    // remove edge that was for cycle detection
    size_t sz = graph[str(v)].size();
    graph[str(v)].remove(str(w));
    // or if removing resource vertex then delete that element of the map
    if (graph[str(v)].size() < sz) {
        return true;
    }
    return false;
//    sz = adj[idx-1].size();
//    adj[idx-1].remove(w); // remove edge from adjacency list
//    pthread_mutex_unlock(&lockRelease);
//    if (adj[idx-1].size() < sz) {
//        return true;
//    }
//    return false;
}

void Graph::releaseResourceVertex(int v, int w) {
    // delete graph[v] if resource is being released
    auto it = find(graph.begin(), graph.end(), v);
    graph.erase(it);
}

// This function is a variation of DFSUtil() in https://www.geeksforgeeks.org/archives/18212
bool Graph::isCyclicUtil(int v, bool visited[], bool *recStack)
{
    if(visited[v] == false)
    {
        // Mark the current node as visited and part of recursion stack
        visited[v] = true;
        recStack[v] = true;

        // Recur for all the vertices adjacent to this vertex
        list<int>::iterator i;
        for(i = adj[v].begin(); i != adj[v].end(); ++i)
        {
            if ( !visited[*i] && isCyclicUtil(*i, visited, recStack) )
                return true;
            else if (recStack[*i])
                return true;
        }

    }
    recStack[v] = false; // remove the vertex from recursion stack
    return false;
}

// Returns true if the graph contains a cycle, else false.
// This function is a variation of DFS() in https://www.geeksforgeeks.org/archives/18212

// may need to edit V since it indicates how large the outer list is
//V may need to be size of graph outer map
bool Graph::isCyclic()
{
    // Mark all the vertices as not visited and not part of recursion stack
    size_t sz = graph.size();
    bool *visited = new bool[sz];
    bool *recStack = new bool[sz];
    for(int i = 0; i < sz; i++)
    {
        visited[i] = false;
        recStack[i] = false;
    }

    // Call the recursive helper function to detect cycle in different
    // DFS trees

    // for element in outer list?? (NOT index)
    for(int i = 0; i < sz; i++)
        if ( !visited[i] && isCyclicUtil(i, visited, recStack))
            return true;

    return false;
}


#endif //PROGRAM3_GRAPH_H
