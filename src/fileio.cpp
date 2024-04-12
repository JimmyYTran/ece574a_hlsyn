#include "fileio.h"

std::vector<std::string> read_file_to_strings(std::string filename)
{
	std::vector<std::string> file_lines;

	std::string line;
	std::ifstream input_file;
	input_file.open(filename);

	if (input_file.is_open())
	{
		while (getline(input_file, line))
		{
			if (line.length() != 0)
			{
				file_lines.push_back(line);
			}
		}
		input_file.close();
	}
	else
	{
		std::cout << "Unable to open file.";
	}

	return file_lines;
}

void write_strings_to_file(std::vector<std::string> lines, std::string filename)
{
	std::ofstream output_file;
	output_file.open(filename);

	if (output_file.is_open())
	{
		for (std::string line : lines)
		{
			output_file << line << "\n";
		}
		output_file.close();
	}
	else
	{
		std::cout << "Unable to open file.";
	}
}