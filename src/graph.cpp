#include "graph.h"

void Graph::do_asap_scheduling()
{
	std::vector<Operation> operations = this->nodes;
	std::vector<Data> line_inputs = this->inputs;
	unsigned int time_index = 1;

	while (!operations.empty())
	{

		for (unsigned int i = 0; i < this->nodes.size(); i++)
		{
			// Get the inputs of the node
			Data temp = operations[i].get_inputs().at(0);
			Data temp2 = operations[i].get_inputs().at(1);
			bool is_Scheduled = false;

			// If temp and temp2 both equal any of the current line_inputs 
			if (std::find(line_inputs.begin(), line_inputs.end(), temp) != line_inputs.end())
			{
				if (std::find(line_inputs.begin(), line_inputs.end(), temp2) != line_inputs.end())
				{
					// Schedule the node at the current time_index
					nodes[i].set_asap_time(time_index);
					is_Scheduled = true;
				}
			}

			// If only temp1 or temp2 or neither are contained in line_inputs, then skip over (no code needed)

			// Push the scheduled node's outputs into line_inputs
			if (is_Scheduled)
			{
				line_inputs.push_back(operations[i].get_output());
			}

			// Pop out the scheduled node's from operations
			operations.erase(std::find(operations.begin(), operations.end(), operations[i]));

		}

		// Increment time_index before moving on to next cycle
		time_index++;

	}

}

void Graph::do_alap_scheduling()
{
	std::vector<Operation> operations = this->nodes;
	std::vector<Data> line_outputs = this->outputs;

	while (!operations.empty())
	{
		for (int i = 0; i < this->nodes.size(); i++)
		{
			if (this->nodes[i].get_alap_time() != -1)
			{
				
			}
		}
	}
}
