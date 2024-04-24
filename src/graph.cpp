#include "graph.h"

void Graph::add_node(Operation node)
{
	this->nodes.push_back(node);
}

void Graph::add_input(Data input)
{
	this->inputs.push_back(input);
}

void Graph::add_output(Data output)
{
	this->outputs.push_back(output);
}

void Graph::do_asap_scheduling()
{
	unsigned int time_index = 1;

	// Iterate through the graph line-by-line (aka node-by-node)
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		Operation current_node = nodes[i];

		// Each node can have at most 2 immediate predecessors
		std::vector<int> predecessors = current_node.get_pred_indices();

		// If the node has no predecessors, then schedule the node at time_index = 1
		if (predecessors.empty())
		{
			nodes[i].set_asap_time(time_index);
		}

		// If the node has predecessors, then access the predecessor nodes
		else
		{
			Operation temp_node = nodes[predecessors.at(0)];
			// Calculate the times when each predecessor's operation finishes executing
			unsigned int predecessor_1_time = temp_node.get_asap_time() + temp_node.get_cycle_delay();
			
			if (predecessors.size() > 1) {

				Operation temp2_node = nodes[predecessors.at(1)];
				unsigned int predecessor_2_time = temp2_node.get_asap_time() + temp2_node.get_cycle_delay();

				// Set the current node's ASAP time_index to which of its two predecessors finishes executing last
				if (predecessor_2_time > predecessor_1_time)
				{
					nodes[i].set_asap_time(predecessor_2_time);
				}
			}

			else
			{
				nodes[i].set_asap_time(predecessor_1_time);
			}
		}
	}

}

void Graph::do_alap_scheduling(unsigned int latency_constraint)
{
	// Iterate through the graph line-by-line (aka node-by-node)
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		Operation current_node = nodes[i];
		unsigned int successor_time_delay = 0;

		// Each node can have an (unlimited?) number of successors
		std::vector<int> successors = current_node.get_succ_indices();

		// If the node has no successors, then schedule the node at latest_time_index, depending on the operation's cycle delay
		if (successors.empty())
		{
			unsigned int latest_index = latency_constraint - current_node.get_cycle_delay();
			nodes[i].set_alap_time(latest_index);
		}

		// If the node has successors, then access successor nodes
		else {

			for (unsigned int j = 0; j < successors.size(); j++)
			{
				Operation temp_node = nodes[successors.at(j)];
				int schedule_time = temp_node.get_alap_time() - temp_node.get_cycle_delay();
				
				if (j == 0)
				{
					successor_time_delay = schedule_time;
				}

				else if (schedule_time < successor_time_delay)
				{
					successor_time_delay = schedule_time;
				}

			}

			nodes[i].set_alap_time(successor_time_delay);

		}

	}

}

void Graph::set_type_distributions()
{
	// TODO: how are we naming operations?
	std::vector<int> addsub_indices;
	std::vector<int> mult_indices;
	std::vector<int> logic_indices;
	std::vector<int> divmod_indices;

	for (int i = 0; i < (this->nodes).size(); i++)
	{
		this->nodes[i].set_op_probs(this->latency_constrant);
	}
}
