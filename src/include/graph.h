#ifndef GRAPH_H
#define GRAPH_H

#include "Operation.h"

#include <vector>
#include <algorithm>

class Graph {
private:
    std::vector<Operation> nodes;
    std::vector<Data> inputs;
    std::vector<Data> outputs;
    int latency_constraint;

    void set_per_operation_type_distribution(std::vector<int> indices);
    double calculate_predecessor_forces(Operation current_node, int current_time);
    double calculate_successor_forces(Operation current_node, int current_time);
public:
    void add_node(Operation node);
    void add_input(Data input);
    void add_output(Data output);
    void do_asap_scheduling();
    void do_alap_scheduling(unsigned int latency_constraint);
    void set_type_distributions();
    void do_fds();

    std::vector<Operation> get_nodes() { return this->nodes; }
    std::vector<Data> get_inputs() { return this->inputs; }
    std::vector<Data> get_outputs() { return this->outputs; }

    int get_latency_constraint() { return this->latency_constraint; }
    void set_latency_constraint(int latency) { this->latency_constraint = latency; }
};

#endif