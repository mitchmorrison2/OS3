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
#include <stdlib.h>


using namespace std;

class Graph
{
    int V; // No. of vertices
    bool isCyclicUtil(string &v, map<string, bool> &visited, map<string, bool> &recStack);
    map<string, list<string>> graph;
    pthread_mutex_t lockRelease;
public:
    Graph(bool isEmpty);
    Graph(int V); // Constructor
    void addEdge(string v, string w); // to add an edge to graph
    bool removeEdge(string v, string w); // to add an edge to graph
    bool edgeExists(string idx, string resource) ;
    void addClaimEdge(string thread, string resource);
    bool vertexExists(string resource) ;
    bool releaseResourceVertex(string v, string w);
    bool updateGraph(string resource);
    bool isCyclic(); // returns true if there is a cycle in this graph
    char* printGraph();
};

Graph::Graph(bool isEmpty) {
    printf("Created graph class\n");
}

Graph::Graph(int V)
{
    this->V = V;
    pthread_mutex_init(&lockRelease, NULL);

}

bool Graph::edgeExists(string idx, string resource) {
// check what resources are being used by this thread and return
// let returned function call free resource on those
    bool found = (find(graph[idx].begin(), graph[idx].end(), resource) != graph[idx].end());
    return found;
}

bool Graph::vertexExists(string resource) {
// check what resources are being used by this thread and return
// let returned function call free resource on those
    for (auto v: graph) {
        if (v.first == resource) {
            return true;
        }
    }
    return false;
}

void Graph::addEdge(string v, string w)
{
    pthread_mutex_lock(&lockRelease);
    // if not causing a cycle or if resource vertex dne then add
    // graph class add vertex
    graph[v].push_back(w);
    pthread_mutex_unlock(&lockRelease);
}

void Graph::addClaimEdge(string thread, string resource) {
    // add claim edge. Key is thread and value is list of all resources it is requesting that have not yet been turned into resource edge
    pthread_mutex_lock(&lockRelease);
    graph[thread].push_back(resource);
    pthread_mutex_unlock(&lockRelease);
}


bool Graph::removeEdge(string v, string w) {
    pthread_mutex_lock(&lockRelease);
    // remove edge that was for cycle detection
    size_t sz = graph[v].size();
    graph[v].remove(w);
    pthread_mutex_unlock(&lockRelease);

    // or if removing resource vertex then delete that element of the map
    if (graph[v].size() < sz) {
        return true;
    }
    return false;


}

bool Graph::releaseResourceVertex(string v, string w) {
    // delete graph[v] if resource is being released
    pthread_mutex_lock(&lockRelease);
    size_t deleted = graph.erase(v);
    pthread_mutex_unlock(&lockRelease);
    return deleted;
}

bool Graph::updateGraph(string resource) {
    // loop through entire graph and see if just removed resource is being requested by another thread at ll and if so create the new vertex
    for (auto & line: graph) {
        for (auto & req: line.second) {
            if (resource == req) {
                removeEdge(line.first, req);
                addEdge(req,line.first);
                return true;
            }
        }
    }
    return false;
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
        for(auto &it: graph[v]) {
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

    return false;
}

char* Graph::printGraph() {
    // how the fuck do I print this thing
    char r2t[1000] = "Resource to Thread ";
    char t2r[1000] = "Thread to Resources ";
    for (auto &v : graph)  {
        try {
            int res = stoi(v.first);
            sprintf(r2t, "%s : (%s, %s) ", r2t, v.first.c_str(), v.second.front().c_str());
            if (v.second.front().c_str() == " " || v.second.front()=="") {
                printf("here");
            }
        }
        catch (exception & e) {
//            printf("Not an int");
            for (int i = 0; i <= V; i++) {
                if (find(v.second.begin(), v.second.end(), to_string(i)) != v.second.end()) {
                    sprintf(t2r, "%s : (%s, %s) ", t2r, v.first.c_str(), to_string(i).c_str());

                }
            }

        }

    }
    sprintf(r2t, "%s \n", r2t);
//    printf(r2t);
    sprintf(t2r, "%s \n", t2r);
//    printf(t2r);
    char bigT[2000] = "";
    sprintf(bigT, "%s %s", r2t, t2r);
    return bigT;
}


#endif //PROGRAM3_GRAPH_H
