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
    bool isCyclicUtil(string &v, map<string, bool> &visited, map<string, bool> &recStack);
//    list<int> *adj; // Pointer to an array containing adjacency lists
//    bool isCyclicUtil(int v, bool visited[], bool *rs); // used by isCyclic()
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
    bool releaseResourceVertex(int v, int w);
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
    bool found = (find(graph[to_string(idx)].begin(), graph[to_string(idx)].end(), to_string(resource)) != graph[to_string(idx)].end());
    return found;
}

bool Graph::vertexExists(int resource) {
// check what resources are being used by this thread and return
// let returned function call free resource on those
    for (auto v: graph) {
        if (v.first == to_string(resource)) {
            return true;
        }
    }
    return false;
}

void Graph::addEdge(int v, int w)
{
    pthread_mutex_lock(&lockRelease);
    // if not causing a cycle or if resource vertex dne then add
    // graph class add vertex
    graph[to_string(v)].push_back(to_string(w));
//    adj[v-1].push_back(w); // Add w to v’s list.
    pthread_mutex_unlock(&lockRelease);
}

void Graph::addClaimEdge(int thread, int resource) {
    // add claim edge. Key is thread and value is list of all resources it is requesting that have not yet been turned into resource edge
    pthread_mutex_lock(&lockRelease);
    graph[to_string(thread)].push_back(to_string(resource));
    pthread_mutex_unlock(&lockRelease);

}


bool Graph::removeEdge(int v, int w) {
    pthread_mutex_lock(&lockRelease);
    v = abs(v);
    // remove edge that was for cycle detection
    size_t sz = graph[to_string(v)].size();
    graph[to_string(v)].remove(to_string(w));
    pthread_mutex_unlock(&lockRelease);

    // or if removing resource vertex then delete that element of the map
    if (graph[to_string(v)].size() < sz) {
        return true;
    }
    return false;

//    sz = adj[idx-1].size();
//    adj[idx-1].remove(w); // remove edge from adjacency list
//    if (adj[idx-1].size() < sz) {
//        return true;
//    }
//    return false;
}

bool Graph::releaseResourceVertex(int v, int w) {
    // delete graph[v] if resource is being released
    pthread_mutex_lock(&lockRelease);
    size_t deleted = graph.erase(to_string(v));
    pthread_mutex_unlock(&lockRelease);
    return deleted;
}

// This function is a variation of DFSUtil() in https://www.geeksforgeeks.org/archives/18212
bool Graph::isCyclicUtil(string &v, map<string, bool> &visited, map<string, bool> &recStack)
{
    if(!visited[v])
    {
        // Mark the current node as visited and part of recursion stack
        visited[v] = true;
        recStack[v] = true;

        // Recur for all the vertices adjacent to this vertex
        for(auto it: graph[v]) {
            if ( !visited[it] && isCyclicUtil(it, visited, recStack) )
                return true;
            else if (recStack[it])
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
//    bool *visited = new bool[sz];
//    bool *recStack = new bool[sz];
    map<string, bool> vis ;
    map<string, bool> rec ;
//    int i = 0;
    for (const auto &pair: graph) {
        vis[pair.first] = false;
        rec[pair.first] = false;
    }
//    for(int i = 0; i < sz; i++)
//    {
//        visited[i] = false;
//        recStack[i] = false;
//    }

    // Call the recursive helper function to detect cycle in different
    // DFS trees
    int i = 0;
    for (auto &pair: graph) {
        if ( !vis[pair.first] && isCyclicUtil(const_cast<string &>(pair.first), vis, rec))
            return true;
        i++;
    }
    // for element in outer list?? (NOT index)
//    int i = 0;
//    for (const auto &pair: graph) {
//        if ( !visited[i] && isCyclicUtil(i, pair.second, visited, recStack))
//            return true;
//        i++;
//    }
//    for(int i = 0; i < sz; i++)

    return false;
}


#endif //PROGRAM3_GRAPH_H
