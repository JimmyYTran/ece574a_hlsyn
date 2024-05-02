#ifndef GRAPH_H
#define GRAPH_H

#include "Operation.h"

#include <vector>
#include <algorithm>

class Graph {
private:
    std::vector<Operation> nodes;
    int base_nodes_size;
    std::vector<Data> inputs;
    std::vector<Data> outputs;
    std::vector<Data> variables;
    int latency_constraint;

    void add_expanded_nodes(int current_node_index);
    void set_per_operation_type_distribution(std::vector<int> indices);
    double calculate_predecessor_forces(Operation current_node, int current_time);
    double calculate_successor_forces(Operation current_node, int current_time);
public:
    Graph();
    Graph(std::vector<Operation> nodes, int latency);
    std::vector<Operation> get_unexpanded_nodes();
    void add_node(Operation node);
    void add_input(Data input);
    void add_output(Data output);
    void add_variable(Data output);
    void link_nodes();
    void do_asap_scheduling();
    void do_alap_scheduling();
    void set_type_distributions();
    void do_fds();
    std::vector<std::vector<Operation>> get_nodes_ordered_by_time();

    std::vector<Operation> get_nodes() { return this->nodes; }

    std::vector<Data> get_inputs() { return this->inputs; }
    void set_inputs(std::vector<Data> inputs) { this->inputs = inputs; }

    std::vector<Data> get_outputs() { return this->outputs; }
    void set_outputs(std::vector<Data> outputs) { this->outputs = outputs; }

    std::vector<Data> get_variables() { return this->variables;  }
    void set_variables(std::vector<Data> variables) { this->variables = variables; }

    int get_latency_constraint() { return this->latency_constraint; }
    void set_latency_constraint(int latency) { this->latency_constraint = latency; }
};

#endif