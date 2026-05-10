/**
 * @file Graph.h
 * @brief Generic adjacency-list graph used as the main representation of the interference graph.
 *
 * This file keeps the graph structure provided in the practical classes and adds only the
 * operations needed by the register-allocation project, namely deep copying, clearing and
 * duplicate-safe bidirectional edges.
 */

#ifndef PROJETO2_GRAPH_H
#define PROJETO2_GRAPH_H

#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
//#include "../data_structures/MutablePriorityQueue.h" // not needed for now

template <class T>
class Edge;

/** @brief Symbolic infinity value used by graph algorithms inherited from the base structure. */
#define INF std::numeric_limits<double>::max()

/************************* Vertex  **************************/

/**
 * @brief Vertex of a directed graph.
 * @tparam T Type stored in each vertex. In this project it is normally Web.
 */
template <class T>
class Vertex {
public:
    /**
     * @brief Builds a vertex storing the given information object.
     * @param in Value to store in the vertex.
     * @complexity O(1).
     */
    explicit Vertex(T in);

    /**
     * @brief Compares vertices by distance, for compatibility with priority-queue based algorithms.
     * @param vertex Vertex to compare against.
     * @return true when this vertex has a smaller stored distance.
     * @complexity O(1).
     */
    bool operator<(Vertex<T> & vertex) const;

    /** @brief Gets the value stored in the vertex. @return Vertex information. @complexity O(1). */
    T getInfo() const;

    /** @brief Gets the outgoing adjacency list. @return Copy of outgoing edge pointers. @complexity O(outdegree). */
    std::vector<Edge<T> *> getAdj() const;

    /** @brief Gets the generic visited flag. @return true if marked as visited. @complexity O(1). */
    bool isVisited() const;

    /** @brief Gets the processing flag used by DFS-style algorithms. @return true if currently processing. @complexity O(1). */
    bool isProcessing() const;

    /** @brief Gets the stored indegree value. @return Indegree counter. @complexity O(1). */
    unsigned int getIndegree() const;

    /** @brief Gets the auxiliary distance value. @return Distance value. @complexity O(1). */
    double getDist() const;

    /** @brief Gets the predecessor/path edge. @return Pointer to path edge, or nullptr. @complexity O(1). */
    Edge<T> *getPath() const;

    /** @brief Gets incoming edges. @return Copy of incoming edge pointers. @complexity O(indegree). */
    std::vector<Edge<T> *> getIncoming() const;

    /** @brief Replaces the stored information object. @param info New vertex information. @complexity O(1). */
    void setInfo(T info);

    /** @brief Sets the visited flag. @param visited New visited value. @complexity O(1). */
    void setVisited(bool visited);

    /** @brief Sets the processing flag. @param processing New processing value. @complexity O(1). */
    void setProcessing(bool processing);

    /** @brief Gets the Tarjan low-link value. @return Low-link value. @complexity O(1). */
    int getLow() const;

    /** @brief Sets the Tarjan low-link value. @param value New low-link value. @complexity O(1). */
    void setLow(int value);

    /** @brief Gets the Tarjan discovery number. @return Discovery number. @complexity O(1). */
    int getNum() const;

    /** @brief Sets the Tarjan discovery number. @param value New discovery number. @complexity O(1). */
    void setNum(int value);

    /** @brief Sets the stored indegree value. @param indegree New indegree counter. @complexity O(1). */
    void setIndegree(unsigned int indegree);

    /** @brief Sets the auxiliary distance value. @param dist New distance. @complexity O(1). */
    void setDist(double dist);

    /** @brief Sets the predecessor/path edge. @param path New path edge pointer. @complexity O(1). */
    void setPath(Edge<T> *path);

    /**
     * @brief Adds an outgoing edge from this vertex to the destination vertex.
     * @param dest Destination vertex pointer.
     * @param w Edge weight.
     * @return Pointer to the newly allocated edge.
     * @complexity O(1).
     */
    Edge<T> * addEdge(Vertex<T> *dest, double w);

    /**
     * @brief Removes all outgoing edges whose destination stores the given value.
     * @param in Destination value to remove.
     * @return true if at least one edge was removed.
     * @complexity O(outdegree * indegree(dest)) because each deleted edge is also removed from the destination incoming list.
     */
    bool removeEdge(T in);

    /**
     * @brief Removes and deletes every outgoing edge of this vertex.
     * @complexity O(outdegree * indegree(dest)) over the deleted edges.
     */
    void removeOutgoingEdges();

    //friend class MutablePriorityQueue<Vertex>;
protected:
    /** @brief Information object stored in the vertex. */
    T info;
    /** @brief Outgoing edges. */
    std::vector<Edge<T> *> adj;

    /** @brief Generic traversal flag. */
    bool visited = false;
    /** @brief Generic recursion-stack flag. */
    bool processing = false;
    /** @brief Auxiliary low-link and discovery numbers. */
    int low = -1, num = -1;
    /** @brief Cached indegree value used by some algorithms. */
    unsigned int indegree = 0;
    /** @brief Auxiliary distance value. */
    double dist = 0;
    /** @brief Auxiliary predecessor edge. */
    Edge<T> *path = nullptr;

    /** @brief Incoming edges. */
    std::vector<Edge<T> *> incoming;

    /** @brief Compatibility field for mutable priority queues and union-find structures. */
    int queueIndex = 0;

    /**
     * @brief Deletes an edge and removes it from the destination incoming list.
     * @param edge Edge to delete.
     * @complexity O(indegree(dest)).
     */
    void deleteEdge(Edge<T> *edge);
};

/********************** Edge  ****************************/

/**
 * @brief Directed weighted edge between two vertices.
 * @tparam T Type stored by the graph vertices.
 */
template <class T>
class Edge {
public:
    /**
     * @brief Builds an edge from origin to destination.
     * @param orig Origin vertex.
     * @param dest Destination vertex.
     * @param w Edge weight.
     * @complexity O(1).
     */
    Edge(Vertex<T> *orig, Vertex<T> *dest, double w);

    /** @brief Gets the destination vertex. @return Destination pointer. @complexity O(1). */
    Vertex<T> * getDest() const;

    /** @brief Gets the edge weight. @return Weight value. @complexity O(1). */
    double getWeight() const;

    /** @brief Gets the selected flag. @return true if selected. @complexity O(1). */
    bool isSelected() const;

    /** @brief Gets the origin vertex. @return Origin pointer. @complexity O(1). */
    Vertex<T> * getOrig() const;

    /** @brief Gets the reverse edge of a bidirectional pair. @return Reverse edge pointer, or nullptr. @complexity O(1). */
    Edge<T> *getReverse() const;

    /** @brief Gets the auxiliary flow value. @return Flow value. @complexity O(1). */
    double getFlow() const;

    /** @brief Sets the selected flag. @param selected New selected value. @complexity O(1). */
    void setSelected(bool selected);

    /** @brief Sets the reverse edge pointer. @param reverse Reverse edge. @complexity O(1). */
    void setReverse(Edge<T> *reverse);

    /** @brief Sets the auxiliary flow value. @param flow New flow value. @complexity O(1). */
    void setFlow(double flow);
protected:
    /** @brief Destination vertex. */
    Vertex<T> * dest;
    /** @brief Edge weight, also usable as a capacity. */
    double weight;

    /** @brief Generic selection flag. */
    bool selected = false;

    /** @brief Origin vertex. */
    Vertex<T> *orig;
    /** @brief Reverse edge in a bidirectional edge pair. */
    Edge<T> *reverse = nullptr;

    /** @brief Auxiliary flow value. */
    double flow = 0;
};

/********************** Graph  ****************************/

/**
 * @brief Generic directed graph represented by adjacency lists.
 *
 * The interference graph is stored as a Graph<Web> with one vertex per live web and
 * bidirectional edges for interference.  The class owns all vertices and edges.
 *
 * @tparam T Type stored in each vertex.
 */
template <class T>
class Graph {
public:
    /** @brief Builds an empty graph. @complexity O(1). */
    Graph() = default;

    /**
     * @brief Deep-copies another graph, preserving vertices and edges.
     * @param other Graph to copy.
     * @complexity O(V * (V + E)) because vertices are searched by value while edges are recreated.
     */
    Graph(const Graph<T>& other);

    /**
     * @brief Replaces this graph with a deep copy of another graph.
     * @param other Graph to copy.
     * @return Reference to this graph.
     * @complexity O(V + E + V * (V + E)), including cleanup and edge recreation.
     */
    Graph<T>& operator=(const Graph<T>& other);

    /** @brief Deletes every vertex, edge and auxiliary matrix owned by the graph. @complexity O(V + E * D), where D is the relevant incoming-list scan cost. */
    ~Graph();

    /**
     * @brief Removes all graph contents and resets auxiliary matrices.
     * @complexity O(V + E * D), where D is the relevant incoming-list scan cost while deleting edges.
     */
    void clear();

    /**
     * @brief Finds a vertex with the given stored value.
     * @param in Value to search for.
     * @return Pointer to the vertex, or nullptr if absent.
     * @complexity O(V).
     */
    Vertex<T> *findVertex(const T &in) const;

    /**
     * @brief Adds a vertex storing the given value.
     * @param in Value to store.
     * @return true if the vertex was added, false if it already existed.
     * @complexity O(V).
     */
    bool addVertex(const T &in);

    /**
     * @brief Removes a vertex and all incident edges.
     * @param in Value stored in the vertex to remove.
     * @return true if a vertex was removed.
     * @complexity O(V + E * D), where D is the relevant incoming-list scan cost for deleted edges.
     */
    bool removeVertex(const T &in);

    /**
     * @brief Adds a directed edge from one stored value to another.
     * @param sourc Origin value.
     * @param dest Destination value.
     * @param w Edge weight.
     * @return true if the edge was added.
     * @complexity O(V + outdegree(source)).
     */
    bool addEdge(const T &sourc, const T &dest, double w);

    /**
     * @brief Removes a directed edge.
     * @param source Origin value.
     * @param dest Destination value.
     * @return true if the edge existed and was removed.
     * @complexity O(V + outdegree(source) * indegree(dest)).
     */
    bool removeEdge(const T &source, const T &dest);

    /**
     * @brief Adds a pair of opposite directed edges between two vertices.
     * @param sourc First endpoint value.
     * @param dest Second endpoint value.
     * @param w Edge weight used for both directions.
     * @return true if at least one direction was added, false if both already existed or an endpoint is missing.
     * @complexity O(V + outdegree(source) + outdegree(dest)).
     */
    bool addBidirectionalEdge(const T &sourc, const T &dest, double w);

    /** @brief Gets the number of vertices. @return Vertex count. @complexity O(1). */
    int getNumVertex() const;

    /** @brief Gets the vertex set. @return Copy of vertex pointers. @complexity O(V). */
    std::vector<Vertex<T> *> getVertexSet() const;

protected:
    /** @brief Owned vertices of the graph. */
    std::vector<Vertex<T> *> vertexSet;

    /** @brief Optional Floyd-Warshall distance matrix inherited from the base structure. */
    double ** distMatrix = nullptr;
    /** @brief Optional Floyd-Warshall path matrix inherited from the base structure. */
    int **pathMatrix = nullptr;

    /**
     * @brief Finds the index of a vertex with the given stored value.
     * @param in Value to search for.
     * @return Index in vertexSet, or -1 if absent.
     * @complexity O(V).
     */
    int findVertexIdx(const T &in) const;
};

/**
 * @brief Deletes an integer matrix allocated as an array of rows.
 * @param m Matrix pointer.
 * @param n Number of rows.
 * @complexity O(n).
 */
void deleteMatrix(int **m, int n);

/**
 * @brief Deletes a double matrix allocated as an array of rows.
 * @param m Matrix pointer.
 * @param n Number of rows.
 * @complexity O(n).
 */
void deleteMatrix(double **m, int n);


/************************* Vertex  **************************/

template <class T>
Vertex<T>::Vertex(T in): info(in) {}

template <class T>
Edge<T> * Vertex<T>::addEdge(Vertex<T> *d, double w) {
    auto newEdge = new Edge<T>(this, d, w);
    adj.push_back(newEdge);
    d->incoming.push_back(newEdge);
    return newEdge;
}

template <class T>
bool Vertex<T>::removeEdge(T in) {
    bool removedEdge = false;
    auto it = adj.begin();
    while (it != adj.end()) {
        Edge<T> *edge = *it;
        Vertex<T> *dest = edge->getDest();
        if (dest->getInfo() == in) {
            it = adj.erase(it);
            deleteEdge(edge);
            removedEdge = true; // allows for multiple edges to connect the same pair of vertices (multigraph)
        }
        else {
            it++;
        }
    }
    return removedEdge;
}

template <class T>
void Vertex<T>::removeOutgoingEdges() {
    auto it = adj.begin();
    while (it != adj.end()) {
        Edge<T> *edge = *it;
        it = adj.erase(it);
        deleteEdge(edge);
    }
}

template <class T>
bool Vertex<T>::operator<(Vertex<T> & vertex) const {
    return this->dist < vertex.dist;
}

template <class T>
T Vertex<T>::getInfo() const {
    return this->info;
}

template <class T>
int Vertex<T>::getLow() const {
    return this->low;
}

template <class T>
void Vertex<T>::setLow(int value) {
    this->low = value;
}

template <class T>
int Vertex<T>::getNum() const {
    return this->num;
}

template <class T>
void Vertex<T>::setNum(int value) {
    this->num = value;
}

template <class T>
std::vector<Edge<T>*> Vertex<T>::getAdj() const {
    return this->adj;
}

template <class T>
bool Vertex<T>::isVisited() const {
    return this->visited;
}

template <class T>
bool Vertex<T>::isProcessing() const {
    return this->processing;
}

template <class T>
unsigned int Vertex<T>::getIndegree() const {
    return this->indegree;
}

template <class T>
double Vertex<T>::getDist() const {
    return this->dist;
}

template <class T>
Edge<T> *Vertex<T>::getPath() const {
    return this->path;
}

template <class T>
std::vector<Edge<T> *> Vertex<T>::getIncoming() const {
    return this->incoming;
}

template <class T>
void Vertex<T>::setInfo(T in) {
    this->info = in;
}

template <class T>
void Vertex<T>::setVisited(bool visited) {
    this->visited = visited;
}

template <class T>
void Vertex<T>::setProcessing(bool processing) {
    this->processing = processing;
}

template <class T>
void Vertex<T>::setIndegree(unsigned int indegree) {
    this->indegree = indegree;
}

template <class T>
void Vertex<T>::setDist(double dist) {
    this->dist = dist;
}

template <class T>
void Vertex<T>::setPath(Edge<T> *path) {
    this->path = path;
}

template <class T>
void Vertex<T>::deleteEdge(Edge<T> *edge) {
    Vertex<T> *dest = edge->getDest();
    // Remove the corresponding edge from the incoming list
    auto it = dest->incoming.begin();
    while (it != dest->incoming.end()) {
        if ((*it)->getOrig()->getInfo() == info) {
            it = dest->incoming.erase(it);
        }
        else {
            it++;
        }
    }
    delete edge;
}

/********************** Edge  ****************************/

template <class T>
Edge<T>::Edge(Vertex<T> *orig, Vertex<T> *dest, double w): orig(orig), dest(dest), weight(w) {}

template <class T>
Vertex<T> * Edge<T>::getDest() const {
    return this->dest;
}

template <class T>
double Edge<T>::getWeight() const {
    return this->weight;
}

template <class T>
Vertex<T> * Edge<T>::getOrig() const {
    return this->orig;
}

template <class T>
Edge<T> *Edge<T>::getReverse() const {
    return this->reverse;
}

template <class T>
bool Edge<T>::isSelected() const {
    return this->selected;
}

template <class T>
double Edge<T>::getFlow() const {
    return flow;
}

template <class T>
void Edge<T>::setSelected(bool selected) {
    this->selected = selected;
}

template <class T>
void Edge<T>::setReverse(Edge<T> *reverse) {
    this->reverse = reverse;
}

template <class T>
void Edge<T>::setFlow(double flow) {
    this->flow = flow;
}

/********************** Graph  ****************************/

template <class T>
int Graph<T>::getNumVertex() const {
    return vertexSet.size();
}

template <class T>
std::vector<Vertex<T> *> Graph<T>::getVertexSet() const {
    return vertexSet;
}

template <class T>
Vertex<T> * Graph<T>::findVertex(const T &in) const {
    for (auto v : vertexSet)
        if (v->getInfo() == in)
            return v;
    return nullptr;
}

template <class T>
int Graph<T>::findVertexIdx(const T &in) const {
    for (unsigned i = 0; i < vertexSet.size(); i++)
        if (vertexSet[i]->getInfo() == in)
            return i;
    return -1;
}

template <class T>
bool Graph<T>::addVertex(const T &in) {
    if (findVertex(in) != nullptr)
        return false;
    vertexSet.push_back(new Vertex<T>(in));
    return true;
}

template <class T>
bool Graph<T>::removeVertex(const T &in) {
    for (auto it = vertexSet.begin(); it != vertexSet.end(); it++) {
        if ((*it)->getInfo() == in) {
            auto v = *it;
            v->removeOutgoingEdges();
            for (auto u : vertexSet) {
                u->removeEdge(v->getInfo());
            }
            vertexSet.erase(it);
            delete v;
            return true;
        }
    }
    return false;
}


template <class T>
void Graph<T>::clear() {
    // Delete the matrices first while we still know the original vertexSet.size()
    deleteMatrix(distMatrix, vertexSet.size());
    deleteMatrix(pathMatrix, vertexSet.size());
    distMatrix = nullptr;
    pathMatrix = nullptr;

    // Delete all edges before deleting vertices.  This matters because
    // deleteEdge updates the destination vertex incoming list.
    for (auto v : vertexSet) {
        v->removeOutgoingEdges();
    }
    for (auto v : vertexSet) {
        delete v;
    }

    // Empty the vector
    vertexSet.clear();
}


template <class T>
bool Graph<T>::addEdge(const T &sourc, const T &dest, double w) {
    auto v1 = findVertex(sourc);
    auto v2 = findVertex(dest);
    if (v1 == nullptr || v2 == nullptr)
        return false;
    for (auto edge : v1->getAdj()) {
        if (edge->getDest()->getInfo() == dest) {
            return false;
        }
    }
    v1->addEdge(v2, w);
    return true;
}

template <class T>
bool Graph<T>::removeEdge(const T &sourc, const T &dest) {
    Vertex<T> * srcVertex = findVertex(sourc);
    if (srcVertex == nullptr) {
        return false;
    }
    return srcVertex->removeEdge(dest);
}

template <class T>
bool Graph<T>::addBidirectionalEdge(const T &sourc, const T &dest, double w) {
    auto v1 = findVertex(sourc);
    auto v2 = findVertex(dest);
    if (v1 == nullptr || v2 == nullptr)
        return false;
    bool existsForward = false;
    bool existsReverse = false;
    for (auto edge : v1->getAdj()) {
        if (edge->getDest()->getInfo() == dest) existsForward = true;
    }
    for (auto edge : v2->getAdj()) {
        if (edge->getDest()->getInfo() == sourc) existsReverse = true;
    }
    if (existsForward && existsReverse) return false;
    Edge<T>* e1 = nullptr;
    Edge<T>* e2 = nullptr;
    if (!existsForward) e1 = v1->addEdge(v2, w);
    if (!existsReverse) e2 = v2->addEdge(v1, w);
    if (e1 != nullptr && e2 != nullptr) {
        e1->setReverse(e2);
        e2->setReverse(e1);
    }
    return true;
}

inline void deleteMatrix(int **m, int n) {
    if (m != nullptr) {
        for (int i = 0; i < n; i++)
            if (m[i] != nullptr)
                delete [] m[i];
        delete [] m;
    }
}

inline void deleteMatrix(double **m, int n) {
    if (m != nullptr) {
        for (int i = 0; i < n; i++)
            if (m[i] != nullptr)
                delete [] m[i];
        delete [] m;
    }
}

template <class T>
Graph<T>::Graph(const Graph<T>& other) {
    for (auto v : other.vertexSet) {
        addVertex(v->getInfo());
    }
    for (auto v : other.vertexSet) {
        for (auto edge : v->getAdj()) {
            addEdge(v->getInfo(), edge->getDest()->getInfo(), edge->getWeight());
        }
    }
}

template <class T>
Graph<T>& Graph<T>::operator=(const Graph<T>& other) {
    if (this == &other) return *this;
    clear();
    for (auto v : other.vertexSet) {
        addVertex(v->getInfo());
    }
    for (auto v : other.vertexSet) {
        for (auto edge : v->getAdj()) {
            addEdge(v->getInfo(), edge->getDest()->getInfo(), edge->getWeight());
        }
    }
    return *this;
}

template <class T>
Graph<T>::~Graph() {
    clear();
}

#endif //PROJETO2_GRAPH_H
