#include "Operation.h"

Operation::Operation()
{
	this->name = "";
	this->asap_time = -1;
	this->alap_time = -1;
	this->fds_time = -1;
}

Operation::Operation(std::string name)
{
	this->name = name;
	this->asap_time = -1;
	this->alap_time = -1;
	this->fds_time = -1;
}

void Operation::add_input(Data input)
{
	this->inputs.push_back(input);
}

int Operation::get_frame_width()
{
	return this->alap_time - this->asap_time + 1;
}

void Operation::set_op_probs(int latency)
{
	for (int time = 1; time <= latency; time++)
	{
		if (time >= this->asap_time && time <= this->alap_time)
		{
			op_probs.push_back(1.0 / this->get_frame_width());
		}
		else
		{
			op_probs.push_back(0.0);
		}
	}
}

void Operation::add_pred_index(int predecessor_index)
{
	this->pred_indices.push_back(predecessor_index);
}

void Operation::add_succ_index(int successor_index)
{
	this->succ_indices.push_back(successor_index);
}

int Operation::get_cycle_delay()
{
	std::string operation_name = this->get_name();

	// Assume operation is not a divider/modulo nor mulitiplication; then use if-statements to update operation_delay
	unsigned int operation_delay = 1;

	// Divider/modulo operation has a cycle delay = 3
	if ((operation_name == "/") || (operation_name == "%"))
	{
		operation_delay = 3;
	}

	// Multiplication operation has a cycle delay = 2
	else if (operation_name == "*")
	{
		operation_delay = 2;
	}

	return operation_delay;

}