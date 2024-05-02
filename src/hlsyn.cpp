#include <iostream>
#include "fileio.h"
#include "graph.h"
#include "parser2.h"

int main(int argc, char* argv[])
{
	std::vector<Operation> op_list;
	Operation node1 = Operation("ADD", "t1 = a + b");
	Data a = Data("a", "Input", 32, false);
	Data b = Data("b", "Input", 32, false);
	Data t1 = Data("t1", "Variable", 32, false);
	node1.add_input(a);
	node1.add_input(b);
	node1.set_output(t1);
	op_list.push_back(node1); // Index 0
	Operation node2 = Operation("ADD", "t2 = t1 + c");
	Data c = Data("c", "Input", 32, false);
	Data t2 = Data("t2", "Variable", 32, false);
	node2.add_input(t1);
	node2.add_input(c);
	node2.set_output(t2);
	op_list.push_back(node2); // Index 1
	Operation node3 = Operation("ADD", "t3 = t2 + d");
	Data d = Data("d", "Input", 32, false);
	Data t3 = Data("t3", "Variable", 32, false);
	node3.add_input(t2);
	node3.add_input(d);
	node3.set_output(t3);
	op_list.push_back(node3); // Index 2
	Operation node4 = Operation("MUL", "t4 = a * e");
	Data e = Data("e", "Input", 32, false);
	Data t4 = Data("t4", "Variable", 32, false);
	node4.add_input(e);
	node4.add_input(a);
	node4.set_output(t4);
	op_list.push_back(node4); // Index 3
	Operation node5 = Operation("ADD", "t5 = t4 + t3");
	Data t5 = Data("t5", "Variable", 32, false);
	node5.add_input(t3);
	node5.add_input(t4);
	node5.set_output(t5);
	op_list.push_back(node5); // Index 4

	Graph HLSM = Graph(op_list, 10);
	HLSM.link_nodes();

	HLSM.do_asap_scheduling();
	HLSM.do_alap_scheduling();
	HLSM.set_type_distributions();
	HLSM.do_fds();

	for (Operation node : HLSM.get_nodes())
	{
		std::cout << node.get_name() << std::endl;
		std::cout << "ASAP Time: " + std::to_string(node.get_asap_time()) << std::endl;
		std::cout << "ALAP Time: " + std::to_string(node.get_alap_time()) << std::endl;
		for (double op_p : node.get_op_probs())
		{
			std::cout << std::to_string(op_p) << " | ";
		}
		std::cout << "FDS Time " + std::to_string(node.get_fds_time()) << std::endl;
		std::cout << std::endl;
	}
	
	return 0;
}