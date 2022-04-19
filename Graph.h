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
#include <utility>
#include <stdlib.h>


using namespace std;

class Graph
{
    int V; // No. of vertices
    bool isCyclicUtil(string &v, map<string, bool> &visited, map<string, bool> &recStack);
    map<string, list<pair<string, int>>> graph;
    pthread_mutex_t lockRelease;
public:
    Graph(int V); // Constructor
    void addEdge(string, string, int); // to add an edge to graph
    bool removeEdge(string, string); // to add an edge to graph
    bool edgeExists(string, string) ;
    void addClaimEdge(string, string, int);
    void changeEdgeType(string, string, int);
    bool vertexExists(string) ;
    bool releaseResourceVertex(string);
    bool updateGraph(string);
    bool isCyclic(); // returns true if there is a cycle in this graph
    char* printGraph();
};


Graph::Graph(int V)
{
    this->V = V;
    pthread_mutex_init(&lockRelease, NULL);

}

bool Graph::edgeExists(string idx, string resource) {
// check what resources are being used by this thread and return
// let returned function call free resource on those
//    bool found = (find(graph[idx].begin(), graph[idx].end(), resource) != graph[idx].end());
//    return found;
    if (graph.find(idx) != graph.end()) {
        for (auto v: graph[idx]) {
            if (v.first == resource) {
                return true;
            }
        }
    }
    return false;
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

void Graph::addEdge(string v, string w, int edgeType=0)
{
    pthread_mutex_lock(&lockRelease);
    // if not causing a cycle or if resource vertex dne then add
    // graph class add vertex
    pair<string, int> pr = make_pair(w, edgeType);
    graph[v].push_back(pr);
    pthread_mutex_unlock(&lockRelease);
}

void Graph::changeEdgeType(string v, string w, int edgeType) {
    pthread_mutex_lock(&lockRelease);
    // this function removes the current edge and replaces it with a new edge type by adding a new pair
    removeEdge(v, w);
    pair<string, int> pr = make_pair(w, edgeType);
    addEdge(v, w, edgeType);
    pthread_mutex_unlock(&lockRelease);
}

void Graph::addClaimEdge(string thread, string resource, int edgeType = 0) {
    // add claim edge. Key is thread and value is list of all resources it is requesting that have not yet been turned into resource edge
    pthread_mutex_lock(&lockRelease);
    pair<string, int> pr = make_pair(resource, edgeType);
    graph[thread].push_back(pr);
    pthread_mutex_unlock(&lockRelease);
}


bool Graph::removeEdge(string v, string w) {
    pthread_mutex_lock(&lockRelease);
    // remove edge that was for cycle detection
    size_t sz = graph[v].size();
    for (auto & r: graph[v]) {
        if (r.first == w) {
            graph[v].remove(r);
            break;
        }
    }
    pthread_mutex_unlock(&lockRelease);

    return graph[v].size() < sz;
}

bool Graph::releaseResourceVertex(string v) {
    // delete graph[v] if resource is being released
    pthread_mutex_lock(&lockRelease);
    size_t deleted = graph.erase(v);
    pthread_mutex_unlock(&lockRelease);
    return deleted;
}

bool Graph::updateGraph(string resource) {
    // loop through entire graph and see if just removed resource is being requested by another thread at ll and if so create the new vertex
    pthread_mutex_lock(&lockRelease);
    bool result = false;
    for (auto & line: graph) {
        if (graph.find(line.first) != graph.end()) {
            for (auto & req: line.second) {
                if (resource == req.first && req.second == 1) {
                    removeEdge(line.first, req.first);
                    addEdge(req.first, line.first);
                    if (isCyclic()) {
                        printf("Graph update causes a cycle. Rejecting new move");
                        removeEdge(req.first, line.first);
                        addClaimEdge(line.first, req.first, 0);
                        continue;
                    }
                    result = true;
                }
            }
        }
    }
    pthread_mutex_unlock(&lockRelease);
    return result;
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
        if (graph.find(v) != graph.end()) {
            for(auto it: graph[v]) {
                if ( !visited[it.first] && isCyclicUtil(it.first, visited, recStack) )
                    return true;
                else if (recStack[it.first])
                    return true;
            }
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

    map<string, bool> vis ;
    map<string, bool> rec ;
    for (const auto &pair: graph) {
        vis[pair.first] = false;
        rec[pair.first] = false;
    }
    // Call the recursive helper function to detect cycle in different
    // DFS trees
    for (auto &pair: graph) {
        if ( !vis[pair.first] && isCyclicUtil(const_cast<string &>(pair.first), vis, rec))
            return true;
    }

    return false;
}

char* Graph::printGraph() {
    // how the fuck do I print this thing
    char r2t[1000] = "Resource to Thread ";
    char t2r[1000] = "Thread to Resources (claim edges) ";
    char req[1000] = "Thread to Resources (request edges) ";

    for (auto v : graph)  {
        try {
            int res = stoi(v.first);
            sprintf(r2t, "%s : (%s, %s) ", r2t, v.first.c_str(), v.second.front().first.c_str());
        }
        catch (exception & e) {
            for (int i = 0; i <= V; i++) {
                pair<string, int> claimPair = make_pair(to_string(i), 0);
                pair<string, int> reqPair = make_pair(to_string(i), 1);

                bool claimExists = (find(v.second.begin(), v.second.end(), claimPair) != v.second.end());
                bool reqExists = (find(v.second.begin(), v.second.end(), reqPair) != v.second.end());

                if (claimExists) {
                    sprintf(t2r, "%s : (%s, %s) ", t2r, v.first.c_str(), to_string(i).c_str());
                }
                else if (reqExists) {
                    sprintf(req, "%s : (%s, %s) ", req, v.first.c_str(), to_string(i).c_str());
                }
            }

        }

    }
    sprintf(r2t, "%s\n", r2t);
    sprintf(t2r, "%s\n", t2r);
    sprintf(req, "%s\n", req);
    char bigT[3000] = "";
    sprintf(bigT, "%s%s%s", r2t, t2r, req);
    return bigT;
}


#endif //PROGRAM3_GRAPH_H
