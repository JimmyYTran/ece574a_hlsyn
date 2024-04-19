#ifndef OPERATION_H
#define OPERATION_H

#include <vector>

#include "Data.h"

class Operation {
private:
	std::string name;
	std::vector<Data> inputs;
	Data output;
public:
	Operation();
	Operation(std::string name);

	void add_input(Data input);

	std::string get_name() { return name; }
	void set_name(std::string name) { this->name = name; }
	std::vector<Data> get_inputs() { return inputs; }
	Data get_output() { return output; }
	void set_output(Data output) { this->output = output; };
};

#endif