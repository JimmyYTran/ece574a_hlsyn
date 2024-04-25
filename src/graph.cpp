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
	std::vector<int> addsub_indices;
	std::vector<int> mult_indices;
	std::vector<int> divmod_indices;
	std::vector<int> logic_indices;

	for (int i = 0; i < (this->nodes).size(); i++)
	{
		// Calculate the operation probabilities for each node, and storing it in each node
		this->nodes[i].set_op_probs(this->latency_constrant);

		std::string operation_name = this->nodes[i].get_name();

		// Organize the nodes by operation, for calculating type distributions later
		if (operation_name == "+" || operation_name == "-")
		{
			addsub_indices.push_back(i);
		}
		else if (operation_name == "*")
		{
			mult_indices.push_back(i);
		}
		else if (operation_name == "/" || operation_name == "%")
		{
			divmod_indices.push_back(i);
		}
		else
		{
			logic_indices.push_back(i);
		}
	}

	// Calculate type distributions for addsub, mult, divmod, and logical operations
	// Each node will have its op probs and an array of type dists based on the operation's type
	set_per_operation_type_distribution(addsub_indices);
	set_per_operation_type_distribution(mult_indices);
	set_per_operation_type_distribution(divmod_indices);
	set_per_operation_type_distribution(logic_indices);
}

void Graph::set_per_operation_type_distribution(std::vector<int> indices)
{
	std::vector<double> type_dist;

	// Starting from 0, since op prob for time 1 is the 0th index of the op prob array
	for (int i = 0; i < this->latency_constrant; i++)
	{
		double current_type_dist = 0;

		// Sum up each node's op prob for current time to get type distribution for current time
		for (int index : indices)
		{
			current_type_dist += this->nodes[index].get_op_probs()[i];
		}

		type_dist.push_back(current_type_dist);
	}

	// Assign the type distribution vector to each node of that operation
	for (int index : indices)
	{
		this->nodes[index].set_type_dists(type_dist);
	}
}

void Graph::do_fds()
{
	Operation current_node;

	// For each node in the graph, perform step 3 of fds
	for (int i = 0; i < this->nodes.size(); i++)
	{
		// If time frame is 1, node can only be scheduled at that time, no need to calculate forces
		if (this->nodes[i].get_asap_time() == this->nodes[i].get_alap_time())
		{
			this->nodes[i].set_fds_time(this->nodes[i].get_asap_time());
		}
		else
		{
			current_node = this->nodes[i];
			double min_total_force = std::numeric_limits<double>::infinity();
			double current_total_force = 0.0;
			int fds_schedule_time = 0;

			// Calculate the forces for each time the node can be scheduled
			for (int time = current_node.get_asap_time(); time <= current_node.get_alap_time(); time++)
			{
				// Calculate the self force when scheduling the node at the current time
				current_total_force = current_node.calculate_self_force(this->latency_constrant, time);

				// Calculate predecessor forces for the node scheduled at the current time
				for (int p_index = 0; p_index < current_node.get_pred_indices().size(); p_index++)
				{
					current_total_force +=
						this->nodes[p_index].calculate_predecessor_force(this->latency_constrant, time);
				}
				
				// Calculate successor forces for the node scheduled at the current time
				for (int s_index = 0; s_index < current_node.get_succ_indices().size(); s_index++)
				{
					current_total_force +=
						this->nodes[s_index].calculate_successor_force(this->latency_constrant, time);
				}

				// Keep track of which schedule time gives the lowest total force
				if (current_total_force < min_total_force)
				{
					min_total_force = current_total_force;
					fds_schedule_time = time;
				}
			}

			// Schedule the node at the time with the lowest total force
			this->nodes[i].set_fds_time(fds_schedule_time);
		}
	}
}
