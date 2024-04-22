#include "graph.h"

void Graph::do_asap_scheduling()
{

}

void Graph::do_alap_scheduling()
{
	std::vector<Operation> operations = this->nodes;
	std::vector<Data> line_outputs = this->outputs;

	while (!operations.empty())
	{
		for (int i = 0; i < this->nodes.size(); i++)
		{
			if (this->nodes[i].get_alap_time() == -1)
			{
				if (std::find(line_outputs.begin(), line_outputs.end(), this->nodes[i].get_output())
					!= line_outputs.end())
				{
					
				}
			}
		}
	}
}
