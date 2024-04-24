#ifndef OPERATION_H
#define OPERATION_H

#include <vector>

#include "Data.h"

class Operation {
private:
	std::string name;
	std::vector<Data> inputs;
	Data output;
	std::vector<double> op_probs;
	std::vector<double> type_dists;
	std::vector<int> pred_indices;
	std::vector<int> succ_indices;
	int asap_time;
	int alap_time;
	int fds_time;
public:
	Operation();
	Operation(std::string name);

	void add_input(Data input);
	int get_frame_width();
	void set_op_probs(int latency);
	void add_pred_index(int predecessor_index);
	void add_succ_index(int successor_index);
	int get_cycle_delay();
	double calculate_self_force(int latency, int self_force_time);

	std::string get_name() { return name; }
	void set_name(std::string name) { this->name = name; }

	std::vector<Data> get_inputs() { return inputs; }

	Data get_output() { return output; }
	void set_output(Data output) { this->output = output; };

	std::vector<double> get_op_probs() { return this->op_probs; }

	std::vector<double> get_type_dists() { return this->op_probs; }
	void set_type_dists(std::vector<double> type_dists) { this->type_dists = type_dists; }

	std::vector<int> get_pred_indices() { return pred_indices; }
	std::vector<int> get_succ_indices() { return succ_indices; }

	int get_asap_time() { return asap_time; }
	void set_asap_time(int asap) { this->asap_time = asap; }

	int get_alap_time() { return alap_time; }
	void set_alap_time(int alap) { this->alap_time = alap; }

	int get_fds_time() { return fds_time; }
	void set_fds_time(int fds) { this->fds_time = fds; }

};

#endif