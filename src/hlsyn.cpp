#include <iostream>
#include <cstdlib>
#include "fileio.h"
#include "graph.h"
#include "parser2.h"
#include "writeVerilog.h"

int main(int argc, char* argv[])
{
	/*
	std::string filename = argv[1];
	int latency = std::atoi(argv[2]);
	std::string output = argv[3];
	*/

	std::string filename = "C:\\Users\\Jimmy\\Documents\\School\\ECE574A\\HW3\\ece574a_hw3\\build\\src\\Debug\\hls_test1.c";
	int latency = 4;
	std::string output = "output.v";

	std::vector<std::string> file_lines = read_file_to_strings(filename);
	if (file_lines.size() == 0)
	{
		return 1;
	}

	auto [operations, ports, error] = parse(file_lines);
	if (!error.empty()) {
		std::cout << error << std::endl;
		return 1;
	}

	for (Operation o : operations) {
		std::cout << o.get_name() << std::endl;
	}

	Graph HLSM = Graph(operations, latency);
	HLSM.set_inputs(ports.inputs);
	HLSM.set_outputs(ports.outputs);
	HLSM.set_variables(ports.variables);

	HLSM.link_nodes();

	HLSM.do_asap_scheduling();
	HLSM.do_alap_scheduling();
	HLSM.set_type_distributions();
	HLSM.do_fds();

	std::vector<std::string> output_strings;
	output_strings.push_back(write_Verilog_code(HLSM));

	write_strings_to_file(output_strings, output);

	for (Operation node : HLSM.get_nodes())
	{
		std::cout << node.get_name() << std::endl;
		std::cout << "ASAP Time: " + std::to_string(node.get_asap_time()) << std::endl;
		std::cout << "ALAP Time: " + std::to_string(node.get_alap_time()) << std::endl;
		std::cout << "Operation probabilities: " << std::endl;
		for (double op_p : node.get_op_probs())
		{
			std::cout << std::to_string(op_p) << " | ";
		}
		std::cout << std::endl;
		std::cout << "Type distributions: " << std::endl;
		for (double ty_d : node.get_type_dists())
		{
			std::cout << std::to_string(ty_d) << " | ";
		}
		std::cout << std::endl;
		std::cout << "FDS Time " + std::to_string(node.get_fds_time()) << std::endl;
		std::cout << std::endl;
	}
	
	return 0;
}