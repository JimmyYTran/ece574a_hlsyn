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