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
    int latency_constrant;
    std::vector<double> type_dist_addsub;
    std::vector<double> type_dist_mult;
    std::vector<double> type_dist_logic;
    std::vector<double> type_dist_divmod;
public:
    void add_node(Operation node);
    void add_input(Data input);
    void add_output(Data output);
    void do_asap_scheduling();
    void do_alap_scheduling(unsigned int latency_constraint);
    void set_type_distributions();

    std::vector<Operation> get_nodes() { return this->nodes; }
    std::vector<Data> get_inputs() { return this->inputs; }
    std::vector<Data> get_outputs() { return this->outputs; }

    int get_latency_constraint() { return this->latency_constrant; }
    void set_latency_constraint(int latency) { this->latency_constrant = latency; }

    std::vector<double> get_type_dist_addsub() { return type_dist_addsub; }
    void set_type_dist_addsub(std::vector<double> type_dists) { this->type_dist_addsub = type_dists; }

    std::vector<double> get_type_dist_mult() { return type_dist_mult; }
    void set_type_dist_mult(std::vector<double> type_dists) { this->type_dist_mult = type_dists; }

    std::vector<double> get_type_dist_logic() { return type_dist_logic; }
    void set_type_dist_logic(std::vector<double> type_dists) { this->type_dist_logic = type_dists; }

    std::vector<double> get_type_dist_divmod() { return type_dist_divmod; }
    void set_type_dist_divmod(std::vector<double> type_dists) { this->type_dist_divmod = type_dists; }
};

#endif