#ifndef GRAPH_H
#define GRAPH_H

#include "Operation.h"

#include <vector>

class Graph {
private:
    std::vector<Operation> nodes;
    int latency_constrant;
public:
    void do_asap_scheduling();
    void do_alap_scheduling();
};

#endif