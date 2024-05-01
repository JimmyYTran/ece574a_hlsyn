#include "graph.h"

Graph::Graph()
{
	this->latency_constraint = -1;
	this->base_nodes_size = 0;
}

Graph::Graph(std::vector<Operation> nodes, int latency)
{
	this->nodes = nodes;
	this->base_nodes_size = nodes.size();
	this->latency_constraint = latency;

	// nodes does not include nested nodes from if, so we need to get those to complete the graph
	for (int i = 0; i < nodes.size(); i++)
	{
		if (this->nodes[i].get_name() == "IF")
		{
			this->add_expanded_nodes(i);
		}
	}
}

// Adds nested operations within if to the graph, while also adding predecessor/successor links
void Graph::add_expanded_nodes(int current_node_index)
{
	Operation current_node = this->nodes[current_node_index];
	std::vector<Operation> current_if_body = current_node.if_body;
	std::vector<Operation> current_else_body = current_node.else_body;

	Operation nested_node;
	int nested_node_graph_index;

	// Adding the operations from within the if statement
	if (!current_if_body.empty())
	{
		for (int if_index = 0; if_index < current_if_body.size(); if_index++)
		{
			nested_node = current_if_body[if_index];

			// Add the node (an operation within the if) to the list of nodes in the graph
			this->nodes.push_back(nested_node);
			nested_node_graph_index = this->nodes.size() - 1;

			// Handling if dependencies and linking nodes together (because the other function only checks ports)
			this->nodes[current_node_index].add_succ_index(nested_node_graph_index);
			this->nodes[nested_node_graph_index].add_pred_index(current_node_index);

			// If this happens to be a nested IF, handle it with recursion
			if (nested_node.get_name() == "IF")
			{
				add_expanded_nodes(nested_node_graph_index);
			}
		}
	}

	// Adding the operations from within the else statement
	if (!current_else_body.empty())
	{
		for (int else_index = 0; else_index < current_else_body.size(); else_index++)
		{
			nested_node = current_if_body[else_index];

			// Add the node (an operation within the else) to the list of nodes in the graph
			this->nodes.push_back(nested_node);
			nested_node_graph_index = this->nodes.size() - 1;

			// Handling if dependencies and linking nodes together (because the other function only checks ports)
			this->nodes[current_node_index].add_succ_index(nested_node_graph_index);
			this->nodes[nested_node_graph_index].add_pred_index(current_node_index);

			// If this happens to be a nested IF, handle it with recursion
			if (nested_node.get_name() == "IF")
			{
				add_expanded_nodes(nested_node_graph_index);
			}
		}
	}
}

std::vector<Operation> Graph::get_unexpanded_nodes()
{
	std::vector<Operation> unexpanded_nodes(this->nodes.begin(), this->nodes.begin() + this->base_nodes_size);
	return unexpanded_nodes;
}

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

void Graph::add_variable(Data variable)
{
	this->variables.push_back(variable);
}

// Links nodes together based on inputs and outputs (IF is already handled in add_expanded_nodes)
void Graph::link_nodes() 
{
	for (int i1 = 0; i1 < this->nodes.size(); i1++) 
	{
		for (int i2 = 0; i2 < this->nodes.size(); i2++) 
		{
			if (i1 == i2) 
			{
				continue;
			}
			std::string n1_name = this->nodes[i1].get_output().get_name();
			for (Data n : this->nodes[i2].get_inputs()) 
			{
				if (n1_name == n.get_name()) 
				{
					this->nodes[i1].add_succ_index(i2);
					this->nodes[i2].add_pred_index(i1);
				}
			}
		}
	}
}

void Graph::do_asap_scheduling()
{
	unsigned int time_index = 1;

	// Iterate through the graph line-by-line (aka node-by-node)
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		Operation current_node = nodes[i];
		unsigned int latest_predecessor_delay = 0;

		// Get all predecessors of the node (nested nodes within if could have 3 predecessors)
		std::vector<int> predecessors = current_node.get_pred_indices();

		// If the node has no predecessors, then schedule the node at time_index = 1
		if (predecessors.empty())
		{
			nodes[i].set_asap_time(time_index);
		}

		// If the node has predecessors, then access the predecessor nodes
		else
		{
			for (unsigned int j = 0; j < predecessors.size(); j++)
			{
				Operation temp_node = nodes[predecessors.at(j)];
				int predecessor_delay = temp_node.get_asap_time() + temp_node.get_cycle_delay();

				if (j == 0 || predecessor_delay > latest_predecessor_delay)
				{
					latest_predecessor_delay = predecessor_delay;
				}

			}

			nodes[i].set_alap_time(latest_predecessor_delay);
		}
	}

}

void Graph::do_alap_scheduling(unsigned int latency_constraint)
{
	// Iterate through the graph line-by-line (aka node-by-node)
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		Operation current_node = nodes[i];
		unsigned int earliest_successor_time = 0;

		// Get all successors of the current node
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
				int successor_alap_time = temp_node.get_alap_time();
				
				if (j == 0 || successor_alap_time < earliest_successor_time)
				{
					earliest_successor_time = successor_alap_time;
				}

			}

			nodes[i].set_alap_time(earliest_successor_time - current_node.get_cycle_delay());

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
		this->nodes[i].set_op_probs(this->latency_constraint);

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
	for (int i = 0; i < this->latency_constraint; i++)
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
				current_total_force += current_node.calculate_self_force(this->latency_constraint, time);

				// Calculate predecessor forces for the node scheduled at the current time
				current_total_force += this->calculate_predecessor_forces(current_node, time);
				
				// Calculate successor forces for the node scheduled at the current time
				current_total_force += this->calculate_successor_forces(current_node, time);

				// Keep track of which schedule time gives the lowest total force
				if (current_total_force < min_total_force)
				{
					min_total_force = current_total_force;
					fds_schedule_time = time;
				}

				// Reset the total force counter before moving on to the next possible schedule time
				current_total_force = 0.0;
			}

			// Schedule the node at the time with the lowest total force
			this->nodes[i].set_fds_time(fds_schedule_time);
		}
	}
}

double Graph::calculate_predecessor_forces(Operation current_node, int current_time)
{
	std::vector<int> pred_indices = current_node.get_pred_indices();
	double pred_force = 0.0;

	// If the node doesn't have predecessors, then there's no predecessor forces
	if (pred_indices.empty())
	{
		return 0.0;
	}

	for (int pred_index : pred_indices)
	{
		Operation prev_node = this->get_nodes()[pred_index];

		// The latest time that the predecessor can start, given the current node's scheduled time
		int prev_node_latest_start_time = current_time - prev_node.get_cycle_delay();

		// Check if scheduling the original node at current_time affects where this predecessor goes.
		// If it doesn't affect where this predecessor would go, there is no predecessor force.
		// Note: Need to pay attention to the previous node's cycle time!
		if (prev_node.get_alap_time() > prev_node_latest_start_time)
		{
			// Calculate all self forces for the times the predecessor can be scheduled, and add them up
			for (int time = prev_node.get_asap_time(); time <= prev_node_latest_start_time; time++)
			{
				pred_force += prev_node.calculate_self_force(this->latency_constraint, time);
			}

			// Recursively calculate forces for any other implicitly scheduled predecessors
			pred_force += calculate_predecessor_forces(prev_node, prev_node_latest_start_time);
		}
	}

	return pred_force;
}

double Graph::calculate_successor_forces(Operation current_node, int current_time)
{
	std::vector<int> succ_indices = current_node.get_succ_indices();
	double succ_force = 0.0;

	// If the node doesn't have successors, then there's no successor forces
	if (succ_indices.empty())
	{
		return 0.0;
	}

	// Find the time that the current operation completes (i.e. time at which it is no longer running)
	int current_node_end_time = current_time + current_node.get_cycle_delay();

	for (int succ_index : succ_indices)
	{
		Operation next_node = this->get_nodes()[succ_index];

		// Check if scheduling the original node at current_time affects where this successor goes.
		// If it doesn't affect where this successor would go, there is no successor force.
		// Note: Need to pay attention to the current node's cycle time!
		if (next_node.get_asap_time() < current_node_end_time)
		{
			// Calculate all self forces for the times the successor can be scheduled, and add them up
			for (int time = current_node_end_time; time <= next_node.get_alap_time(); time++)
			{
				succ_force += next_node.calculate_self_force(this->latency_constraint, time);
			}

			// Recursively calculate forces for any other implicitly scheduled successors
			succ_force += calculate_successor_forces(next_node, current_node_end_time);
		}
	}

	return succ_force;
}

// Function that gives a vector of vectors ordered by when they're scheduled
std::vector<std::vector<Operation>> Graph::get_nodes_ordered_by_time()
{
	std::vector<std::vector<Operation>> ordered_nodes;
	std::vector<Operation> current_time_nodes;

	for (int time = 1; time <= this->latency_constraint; time++)
	{
		for (Operation node : this->nodes)
		{
			if (node.get_fds_time() == time)
			{
				current_time_nodes.push_back(node);
			}
		}

		ordered_nodes.push_back(current_time_nodes);
		current_time_nodes.clear();
	}
	
	return ordered_nodes;
}