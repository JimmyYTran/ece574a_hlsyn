#include "parser2.h"

std::tuple<std::vector<Operation>, Ports, std::string> parse(std::vector<std::string> lines) {
    lines = cleanup_lines(lines);
    int parsed_lines = 0;
    auto [ports, e1] = parse_ports(lines, parsed_lines);
    if (!e1.empty()) {
        return std::make_tuple(std::vector<Operation>(), ports, e1); 
    }
    auto [operations, e2] = parse_main(lines, ports, parsed_lines);
	if (!e1.empty()) {
        return std::make_tuple(operations, ports, e2);
    }
	for (auto o : operations) {
		if (o.get_name().compare("ERROR") == 0) {
			return std::make_tuple(operations, ports, "Found an error");
		}
	}
    return std::make_tuple(operations, ports, e2);
}

std::tuple<std::vector<Operation>, std::string> parse_main(std::vector<std::string> lines, Ports ports, int& parsed_lines) {

    int index = parsed_lines;
    std::vector<Operation> operations;

    while (index < lines.size()) {
        if (lines[index].find("if (") != std::string::npos) { // if case
			auto [operation, error] = parse_if_statement(lines, ports, index);
			if (error.empty()) {
                operations.push_back(operation);
            } else {
                return std::make_tuple(operations, error);
            }
		} else { // single line case
			auto [operation, error] = parse_single_line(lines, ports, index);
			if (error.empty()) {
                operations.push_back(operation);
            } else {
                return std::make_tuple(operations, error);
            }
		}
	}
    return std::make_tuple(operations, "");
}

std::tuple<Operation, std::string> parse_if_statement(std::vector<std::string> lines, Ports ports, int& parsed_lines) {

    Operation if_operation = Operation("IF");

	std::string first_line = lines[parsed_lines];
	std::vector<std::string> split_line = split_string(first_line);

	if (split_line.size() != 5) {
		return std::make_tuple(Operation(), "Split line size in line '" + first_line + "' != 5\n");
	}
	if (split_line[0].compare("if") != 0 || split_line[1].compare("(") != 0 || split_line[3].compare(")") != 0 || split_line[4].compare("{") != 0) {
		return std::make_tuple(Operation(), "Split line size in line '" + first_line + "' has no if, no (, no ) or no {\n");
	}
	std::optional<Data> condition = ports.get_data_with_name(split_line[2]);
	if (!condition.has_value()) {
		return std::make_tuple(Operation(), "Condition in if statement in line '" + first_line + "' is not present in ports declaration.\n");
	}

    if_operation.if_condition = condition.value();
	parsed_lines += 1;

	// parse if body
	std::vector<Operation> if_body;
	while (parsed_lines < lines.size() && lines[parsed_lines].find("}") == std::string::npos) {
		if (lines[parsed_lines].find("if (") != std::string::npos) { // nested if statement
            auto [operation, error] = parse_if_statement(lines, ports, parsed_lines);
            if (error.empty()) {
                if_operation.if_body.push_back(operation);
            } else {
                return std::make_tuple(if_operation, error);
            }
        }
        else {
            auto [operation, error] = parse_single_line(lines, ports, parsed_lines);
            if (error.empty()) {
                if_operation.if_body.push_back(operation);
            } else {
                return std::make_tuple(if_operation, error);
            }
        }
	}
	if (parsed_lines < lines.size() && lines[parsed_lines].find("}") != std::string::npos) {
		parsed_lines += 1;
	}

	if (parsed_lines < lines.size() && lines[parsed_lines].find("else {") != std::string::npos) { // not at end yet
		parsed_lines += 1;
		while (parsed_lines < lines.size() && lines[parsed_lines].find("}") == std::string::npos) {
            if (lines[parsed_lines].find("if (") != std::string::npos) { // nested if statement
                auto [operation, error] = parse_if_statement(lines, ports, parsed_lines);
                if (error.empty()) {
                    if_operation.else_body.push_back(operation);
                } else {
                    return std::make_tuple(if_operation, error);
                }
            }
            else {
                auto [operation, error] = parse_single_line(lines, ports, parsed_lines);
                if (error.empty()) {
                    if_operation.else_body.push_back(operation);
                } else {
                    return std::make_tuple(if_operation, error);
                }
            }
        }
	}
	if (parsed_lines < lines.size() && lines[parsed_lines].find("}") != std::string::npos) {
		parsed_lines += 1;
	}
	
	return std::make_tuple(if_operation, "");

}

std::tuple<Operation, std::string> parse_single_line(std::vector<std::string> lines, Ports ports, int& parsed_lines) {
    std::vector<std::string> split_line = split_string(lines[parsed_lines]);
    parsed_lines += 1;
    std::string module_type = determine_module(split_line);
    Operation current_op = parse_line_to_operation(split_line, module_type, ports.get_all_ports());
    return std::make_tuple(current_op, "");
}

std::tuple<Ports, std::string> parse_ports(std::vector<std::string> lines, int& parsed_lines) {
    Ports ports;
    int parsed = 0;
    for (std::string l : lines) {
        if (l.find("=") != std::string::npos || l.find("{") != std::string::npos) {
            break;
        }

        std::vector<std::string> split_line = split_string(l);
        std::string datatype = "";
        int datawidth = 1;			// datawidth is 1 unless otherwise specified
        bool is_signed = false;		// port is unsigned unless otherwise specified
        std::string port_name = "";

        // Determine the type of port (input/output/variable)
        datatype = split_line[0];

        // Determine if the port is signed
        if (split_line[1].at(0) != 'U')
        {
            is_signed = true;
        }

        // Determine the datawidth of ports defined on this line from the datatype
        std::size_t it = split_line[1].find_first_of("0123456789");
        if (it != std::string::npos)
        {
            datawidth = std::stoi(split_line[1].substr(it));
        }

        // Appending the inputs/outputs/wires
        for (int i = 2; i < split_line.size(); i++)
        {

            port_name = split_line[i].substr(0, split_line[i].find(","));

            if (datatype.compare("input") == 0 && ports.name_in_inputs(port_name) == -1) {
                ports.inputs.push_back(Data(port_name, datatype, datawidth, is_signed));

            }
            else if (datatype.compare("output") == 0 && ports.name_in_outputs(port_name) == -1) {
                ports.outputs.push_back(Data(port_name, datatype, datawidth, is_signed));

            }
            else if (datatype.compare("variable") == 0 && ports.name_in_variables(port_name) == -1) {
                ports.variables.push_back(Data(port_name, datatype, datawidth, is_signed));

            }
        }
        parsed += 1;
    }
    parsed_lines += parsed;
    return std::make_tuple(ports, "");
}

// Extra functions

std::string strip_comment_from_line(std::string line)
{
	std::size_t found = line.find("//");
	return line.substr(0, found);
}

bool has_only_spaces(const std::string str) {
	return str.find_first_not_of(' ') == std::string::npos;
}

std::vector<std::string> cleanup_lines(std::vector<std::string> lines) {
    std::vector<std::string> clean;
    for (std::string l : lines) {
		std::string l2 = strip_comment_from_line(l);
        l2.erase(std::remove(l2.begin(), l2.end(), '\t'), l2.end());
		if (has_only_spaces(l2)) {
			continue;
		}
        if (!l2.empty()) {
            clean.push_back(l2);
        }
    }
    return clean;
}

std::vector<std::string> split_string(std::string line)
{
	std::vector<std::string> split_line;
	std::istringstream iss(line);
	std::string word;

	while (std::getline(iss, word, ' '))
	{
		if (word.size() != 0)
		{
			split_line.push_back(word);
		}
	}

	return split_line;
}

std::string determine_module(std::vector<std::string> split_line)
{
	if (split_line.size() < 4)
	{
		return "REG";
	}

	std::string op = split_line[3];

	if (op.compare("+") == 0)
	{
		return split_line[4].compare("1") == 0 ? "INC" : "ADD";
	}
	else if (op.compare("-") == 0)
	{
		return split_line[4].compare("1") == 0 ? "DEC" : "SUB";
	}
	else
	{
		auto it = std::find(OPERATION_SYMBOLS.begin(), OPERATION_SYMBOLS.end(), op);
		if (it != OPERATION_SYMBOLS.end())
		{
			int index = it - OPERATION_SYMBOLS.begin();
			return MODULES[index];
		}
		else
		{
			return "ERROR";
		}
	}
}

Operation parse_line_to_operation
(std::vector<std::string> split_line, std::string module_type, std::vector<Data> ports)
{
	// Check for missing ports; if missing, there's an error
	int port_num = split_line.size();
	if (module_type.compare("INC") == 0 || module_type.compare("DEC") == 0)
	{
		port_num -= 2;
	}
	for (int i = 0; i < port_num; i = i + 2)
	{
		if (find_port(ports, split_line[i]) == -1)
		{
			return Operation("ERROR");
		}
	}

	// Construct a new operation from the module_type
	Operation new_op = Operation(module_type);
	int input_index;

	// Set the output for the new operation	
	int output_index = find_port(ports, split_line[0]);
	new_op.set_output(ports[output_index]);

	// Add the inputs for the new operation
	if (module_type.compare("REG") == 0)
	{
		input_index = find_port(ports, split_line[2]);
		new_op.add_input(ports[input_index]);

		// For Clk and Rst, need to add to both new operation and ports (so we can track all inputs)
		new_op.add_input(ports[find_port(ports, "Clk")]);
		new_op.add_input(ports[find_port(ports, "Rst")]);
	}
	else if (module_type.compare("MUX2x1") == 0)
	{
		// MUX inputs need to be read in a slightly different order
		for (int i = 4; i < split_line.size(); i = i + 2)
		{
			input_index = find_port(ports, split_line[i]);
			new_op.add_input(ports[input_index]);
		}
		// Adding the select input
		input_index = find_port(ports, split_line[2]);
		new_op.add_input(ports[input_index]);
	}
	else
	{
		for (int i = 2; i < port_num; i = i + 2)
		{
			input_index = find_port(ports, split_line[i]);
			new_op.add_input(ports[input_index]);
		}
	}

	return new_op;
}

int find_port(std::vector<Data> ports, std::string new_port_name)
{
	for (int i = 0; i < ports.size(); i++)
	{
		if (ports[i].get_name().compare(new_port_name) == 0)
		{
			return i;
		}
	}

	return -1;
}