#include <iostream>
#include "fileio.h"
#include "graph.h"
#include "parser2.h"

int main(int argc, char* argv[])
{
	std::string filename = "E:\\STORAGE\\UofA\\Y5S2\\ECE574a\\ece574a_hlsyn\\assignment3_testfiles_new\\error tests\\error1.c";

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

	return 0;
}