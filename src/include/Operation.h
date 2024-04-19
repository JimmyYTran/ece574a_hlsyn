#ifndef OPERATION_H
#define OPERATION_H

#include <vector>

#include "Data.h"

class Operation {
private:
	int id;
	std::string name;
	std::vector<Data> inputs;
	Data output;
	int asap_time;
	int alap_time;
public:
	Operation();
	Operation(std::string name);

	void add_input(Data input);

	int get_id() { return id; }
	std::string get_name() { return name; }
	void set_name(std::string name) { this->name = name; }
	std::vector<Data> get_inputs() { return inputs; }
	Data get_output() { return output; }
	void set_output(Data output) { this->output = output; };
	int get_asap_time() { return asap_time; }
	void set_asap_time(int asap) { this->asap_time = asap; }
	int get_alap_time() { return alap_time; }
	void set_alap_time(int alap) { this->alap_time = alap; }
};

#endif