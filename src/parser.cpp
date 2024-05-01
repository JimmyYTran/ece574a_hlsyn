#include "parser.h"

/*
* Convert lines from a behavioral netlist to lines in structural Verilog.
* Additionally, keeps track of each operation and its inputs and outputs.

* lines: vector of strings containing lines read from a netlist file.
* filename: string containing the filename, used for creating a circuit name.
* operations: vector of Operations to keep track of each operation.
* Returns a vector of strings, with each string being a line of Verilog.
*/
std::vector<std::string> parse_netlist_lines
(std::vector<std::string> lines, std::string filename, std::vector<Operation>& operations)
{
	std::vector<std::string> verilog_lines;

	for (int i = 0; i < lines.size(); i++)
	{
		lines[i] = strip_comment_from_line(lines[i]);
	}

	lines = remove_empty_lines(lines);

	int line_index = 0;
	std::vector<Data> ports;

	// For registers, need Clk and Rst
	Data clk_in = Data("Clk", "input", 1, false);
	Data rst_in = Data("Rst", "input", 1, false);
	ports.push_back(clk_in);
	ports.push_back(rst_in);
	verilog_lines.push_back("\tinput Clk, Rst;");

	// Write the port declarations, while also keeping track of inputs/outputs/wires
	while (line_index < lines.size() && lines[line_index].find("=") == std::string::npos)
	{
		verilog_lines.push_back("\t" + create_port_declaration_from_line(lines[line_index], ports));
		line_index++;
	}

	int processed_lines = 0;
	std::vector<std::string> processed = parse_line(lines, line_index, ports, operations, processed_lines);
	if (processed.size() == 0) { // error case
		return processed;
	}
	verilog_lines.insert(verilog_lines.end(), processed.begin(), processed.end()); // extend verilog lines with the new processed

	//std::string circuit_name = filename.substr(0, filename.find('.'));
	//verilog_lines.insert(verilog_lines.begin(), write_module_definition(ports, circuit_name));
	//verilog_lines.insert(verilog_lines.begin(), "`timescale 1ns / 1ps");
	//verilog_lines.push_back("endmodule");

	return verilog_lines;
}

std::vector<std::string> parse_line(std::vector<std::string> lines, int current_line, std::vector<Data> ports, std::vector<Operation>& operations, int& num_processed_lines) {

	int line_index = current_line;
	std::vector<std::string> verilog_lines;

	while (line_index < lines.size())
	{
		//if (lines[line_index].find('}') != std::string::npos) {
		//	line_index += 1;
		//	break;
		//}
		// the current line is the start of an if
		if (lines[line_index].find('{') != std::string::npos) {
			int lines_processed = 0;
			std::vector<Operation> ops;
			std::string verilog_if = parse_if_statement(lines, line_index, ports, ops, lines_processed);
			operations.insert(operations.end(), ops.begin(), ops.end());
			if (verilog_if.compare("ERROR") == 0)
			{
				std::cout << "HERE Found error.\n";
				return std::vector<std::string>();
			}
			verilog_lines.push_back(verilog_if);
			line_index += lines_processed;
			std::cout << "DDDDDDDDDDD" << std::endl;
		} else {
			std::string module_line = create_module_instance_from_line(lines[line_index], line_index, ports, operations);
			std::cout <<  "O.\n";
			if (module_line.compare("ERROR") == 0)
			{
				std::cout << lines[line_index] << "   HERE1 Found error.\n";
				return std::vector<std::string>();
			}

			verilog_lines.push_back("\t" + module_line);
			line_index++;
		}
	}
	num_processed_lines = line_index - current_line;
	return verilog_lines;
}

std::string parse_if_statement(std::vector<std::string> lines, int line_index, std::vector<Data> ports, std::vector<Operation>& operations, int& lines_processed) {

	std::string verilog;
	std::string first_line = lines[line_index];
	std::vector<std::string> split_line = split_string(first_line);

	if (split_line.size() != 5) {
		return "ERROR";
	}
	if (split_line[0].compare("if") != 0 || split_line[1].compare("(") != 0 || split_line[3].compare(")") != 0 || split_line[4].compare("{") != 0) {
		return "ERROR";
	}
	Data condition;
	for (int i = 0; i < ports.size(); i++) {
		if (ports[i].get_name().compare(split_line[2]) == 0) {
			condition = ports[i];
		}
	}
	if (condition.get_name().size() == 0) {
		return "ERROR";
	}
	int index = 1;
	// parse if body
	std::vector<Operation> if_body;
	while (index + line_index < lines.size() && lines[index+line_index].find("}") == std::string::npos) {
		int l = 0;
		std::cout << "asd" << lines[index + line_index] << std::endl;
		std::vector<std::string> parsed = parse_line(lines, index + line_index, ports, if_body, l);
		if (parsed.size() == 0) {
			std::cout << "q" << lines[index + line_index] << std::endl;
			return "ERROR";
		}
		for (std::string p : parsed) {
			verilog += (p + "\n");
		}
		index += l;
	}
	std::vector<Operation> else_body;
	if (index + line_index < lines.size()) { // not at end yet
		std::vector<std::string> split_line = split_string(lines[index + line_index]);
		if (split_line.size() == 2 && split_line[0].compare("else") == 0 && split_line[1].compare("{") == 0) { // else body found
			index += 1;
			int l = 0;
			while (index + line_index < lines.size() && lines[index+line_index].find("}") == std::string::npos) {
				std::vector<std::string> parsed = parse_line(lines, index + line_index, ports, else_body, l);
				if (parsed.size() == 0) {
					return "ERROR";
				}
				for (std::string p : parsed) {
					verilog += (p + "\n");
				}
				index += l;
			}
		}
	}
	std::cout << "Done parsing if" << std::endl;
	
	// std::string lines_used;
	// for (int j = line_index; j < line_index + index; j++) {
	// 	lines_used += (lines[j] + "\n");
	// }

	Operation if_operation = Operation("IF");
	if_operation.if_condition = condition;
	if_operation.if_body = if_body;
	if_operation.else_body = else_body;
	lines_processed += index;
	return verilog;
}

/*
* Removes comments from a line from a behavioral netlist.
*
* line: a string containing a line from a netlist file.
* Returns the string without comments.
*/
std::string strip_comment_from_line(std::string line)
{
	std::size_t found = line.find("//");
	return line.substr(0, found);
}

/*
* Removes empty lines (to catch any lines that were commented out).
*
* lines: vector of strings containing lines from a netlist.
* Returns the vector of strings with empty strings removed.
*/
std::vector<std::string> remove_empty_lines(std::vector<std::string> lines)
{
	std::vector<std::string> output_lines;

	for (std::string line : lines)
	{
		if (!line.empty())
		{
			output_lines.push_back(line);
		}
	}

	return output_lines;
}

std::string write_module_definition(std::vector<Data> ports, std::string circuit_name)
{
	Data current_port;
	std::string module_def = "module " + circuit_name + "(";

	for (int i = 0; i < ports.size(); i++)
	{
		current_port = ports[i];

		if (current_port.get_datatype().compare("input") == 0 ||
			current_port.get_datatype().compare("output") == 0)
		{
			module_def += current_port.get_name() + ", ";
		}
	}

	// Get rid of the last comma and end module header
	module_def = module_def.substr(0, module_def.find_last_of(",")) + ");";
	return module_def;
}

/*
* Creates a Verilog port declaration (input, output, wire) from a line from a netlist.
*
* line: a string containing a line from a netlist file.
* ports: a vector of Data containing inputs, outputs, and wires (used later).
* Returns a port declaration written in Verilog.
*/
std::string create_port_declaration_from_line(std::string line, std::vector<Data>& ports)
{
	std::string veri_line = "";
	std::vector<std::string> split_line = split_string(line);
	std::string datatype = "";
	int datawidth = 1;			// datawidth is 1 unless otherwise specified
	bool is_signed = false;		// port is unsigned unless otherwise specified
	std::string port_name = "";

	// Determine the type of port (input/output/wire)
	datatype = split_line[0];
	if (datatype.compare("register") == 0)
	{
		veri_line += "reg";
	}
	else
	{
		veri_line += datatype;
	}

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
		if (datawidth > 1)
		{
			veri_line += " [" + std::to_string(datawidth - 1) + ":0]";
		}
	}

	// Appending the inputs/outputs/wires
	for (int i = 2; i < split_line.size(); i++)
	{
		veri_line += " " + split_line[i];

		port_name = split_line[i].substr(0, split_line[i].find(","));

		// Only add a new port if it hasn't already been added
		if (find_port(ports, port_name) == -1)
		{
			ports.push_back(Data(port_name, datatype, datawidth, is_signed));
		}
	}

	return veri_line + ";";
}

/*
* Creates an instance of a module in Verilog from a line from a netlist.
*
* line: a string containing a line from a netlist file.
* line_num: line number in netlist, used to create a unique module instance name.
* ports: vector of Data, containing all inputs/outputs/wires.
* operations: vector of Operations, for keeping track of modules and their inputs/outputs.
* Returns a module instantiation written in Verilog.
*/
std::string create_module_instance_from_line
(std::string line, int line_num, std::vector<Data> ports, std::vector<Operation>& operations)
{
	std::cout << "IMHERE " << line << std::endl;
	std::string veri_line = "";
	std::vector<std::string> split_line = split_string(line);

	// Determine the module to instantiate
	std::string module_type = determine_module(split_line);
	if (module_type.compare("ERROR") == 0)
	{
		return "ERROR";
	}

	// Keep track of the module, then concatenate to the verilog line if all inputs found
	Operation current_op = parse_line_to_operation(split_line, module_type, ports);
	if (current_op.get_name().compare("ERROR") == 0)
	{
		return "ERROR";
	}
	operations.push_back(current_op);

	if (current_op.get_name().compare("INC") == 0 || current_op.get_name().compare("DEC") == 0)
	{
		if (current_op.get_inputs().at(0).get_is_signed())
		{
			veri_line += "S";
		}
	}
	else if (current_op.get_inputs().at(0).get_is_signed() || current_op.get_inputs().at(1).get_is_signed())
	{
		if ((current_op.get_name().compare("MUX2x1") != 0) && 
			(current_op.get_name().compare("REG") != 0) &&
			(current_op.get_name().compare("SHL") != 0) &&
			(current_op.get_name().compare("SHR") != 0))
		{
			veri_line += "S";
		}
	}

	veri_line += module_type.substr(0, module_type.find_first_of("><="));

	int op_datawidth = 0;

	// If current operation is comparator, then find max datawidth between inputs
	if ((current_op.get_name().compare("COMP") == 0) || (current_op.get_name().compare("COMP<") == 0) || (current_op.get_name().compare("COMP==") == 0))
	{
		std::string concatenate = "";
		int datawidth_diff = current_op.get_inputs().at(0).get_datawidth() - current_op.get_inputs().at(1).get_datawidth();
		if (datawidth_diff < 0)
		{
			op_datawidth = current_op.get_inputs().at(1).get_datawidth();
		}
		else if (datawidth_diff > 0)
		{
			op_datawidth = current_op.get_inputs().at(0).get_datawidth();
		}
		else
		{
			op_datawidth = current_op.get_inputs().at(0).get_datawidth();
		}
	}

	else // If not a comparator, find datawidth of output
	{
		op_datawidth = current_op.get_output().get_datawidth();

	}

	std::string datawidth_call = " #(.DATAWIDTH(" + std::to_string(op_datawidth) + "))";
	veri_line += datawidth_call;

	// Generate a unique name for the instantiation of the module
	veri_line += " " + module_type.substr(0, 1) + std::to_string(line_num);

	// Generate list of inputs/outputs/wires
	veri_line += write_input_list(current_op);

	veri_line += ";";
	return veri_line;
}

/*
* Determines which module should be instantiated based on a line from the netlist.
*
* split_line: a vector of strings containing a line split from a netlist file.
* Returns a string corresponding to the correct module.
*/
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

/*
* Create an Operation instance with inputs and outputs from a line split from a netlist.
*
* split_line: vector of strings from a netlist split line.
* module_type: string representing the operation type.
* ports: vector of Data containing all inputs and outputs for the netlist.
* Returns an Operation instance.
*/
Operation parse_line_to_operation
(std::vector<std::string> split_line, std::string module_type, std::vector<Data>& ports)
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
		Data clk_input = Data("Clk", "input", 1, false);
		ports.push_back(clk_input);
		new_op.add_input(clk_input);
		Data rst_input = Data("Rst", "input", 1, false);
		ports.push_back(rst_input);
		new_op.add_input(rst_input);
	}
	else if (module_type.compare("MUX2x1") == 0)
	{
		// MUX inputs need to be read in a slightly different order
		for (int i = 4; i < split_line.size(); i = i + 2)
		{
			input_index = find_port(ports, split_line[i]);
			ports.push_back(ports[input_index]);
			new_op.add_input(ports[input_index]);
		}
		// Adding the select input
		input_index = find_port(ports, split_line[2]);
		ports.push_back(ports[input_index]);
		new_op.add_input(ports[input_index]);
	}
	else
	{
		for (int i = 2; i < port_num; i = i + 2)
		{
			input_index = find_port(ports, split_line[i]);
			ports.push_back(ports[input_index]);
			new_op.add_input(ports[input_index]);
		}
	}

	return new_op;
}

/*
* Writes a list of inputs for the module.
*
* operation: Operation instance with associated name, inputs, and outputs.
* Returns a string for the input list.
*/
std::string write_input_list(Operation operation)
{
	std::vector<Data> op_inputs = operation.get_inputs();
	Data op_output = operation.get_output();

	std::string input_list = "(";

	// If the operation is a comparator
	if ((op_output.get_name().compare("COMP>") == 0) || (op_output.get_name().compare("COMP<") == 0) || (op_output.get_name().compare("COMP==") == 0))
	{
		std::string concatenate = "";

		int datawidth_diff = op_inputs.at(0).get_datawidth() - op_inputs.at(1).get_datawidth();
		if (datawidth_diff < 0)
		{
			datawidth_diff = datawidth_diff * -1;
			concatenate = "{{" + std::to_string(datawidth_diff) + "{b[" + std::to_string(datawidth_diff - 1) + "]}}," + op_inputs.at(0).get_name() + "}";
		}
		else if (datawidth_diff > 0)
		{
			concatenate = "{{" + std::to_string(datawidth_diff) + "{b[" + std::to_string(datawidth_diff - 1) + "]}}," + op_inputs.at(1).get_name() + "}";
		}
		input_list += concatenate + ", ";
	}

	else if (operation.get_name().compare("REG") != 0) // If operation is not a comparator nor a REG, add all of the inputs to the input list first
	{
		for (Data op_in : op_inputs)
		{
			int datawidth_diff = op_output.get_datawidth() - op_in.get_datawidth();
			if (datawidth_diff > 0) // output datawidth is greater than input datawidth
			{
				std::string concatenate = "{{" + std::to_string(datawidth_diff) + "{b[" + std::to_string(datawidth_diff - 1) + "]}}," + op_in.get_name() + "}, ";
				input_list += concatenate;
			}
			else
			{
				input_list += op_in.get_name() + ", ";
			}
		}
	}

	else
	{
		for (Data op_in : op_inputs)
		{
			input_list += op_in.get_name() + ", ";
		}
	}

	// Add the output, but be extra careful with COMP
	if (operation.get_name().compare("COMP>") == 0)
	{
		input_list += op_output.get_name() + ", _, _";
	}
	else if (operation.get_name().compare("COMP<") == 0)
	{
		input_list += "_, " + op_output.get_name() + ", _";
	}
	else if (operation.get_name().compare("COMP==") == 0)
	{
		input_list += "_, _, " + op_output.get_name();
	}
	else
	{
		input_list += op_output.get_name();
	}

	input_list += ")";
	return input_list;
}

/*
* Splits a line from a netlist, using a single space as the delimiter.
*
* line: a string containing a line from a netlist file.
* Returns the result of the splitting the line by spaces as a vector of strings.
*/
std::vector<std::string> split_string(std::string line)
{
	std::vector<std::string> split_line;
	std::istringstream iss(line);
	std::string word;

	while (std::getline(iss, word, ' '))
	{
		if (word.size() != 0)
		{
			word.erase(std::remove(word.begin(), word.end(), '\t'), word.end());
			split_line.push_back(word);
		}
	}

	return split_line;
}

/*
* Check if the port (input/output/wire) has already been tracked in the vector of ports.
*
* ports: a vector of Data corresponding to ports (inputs/outputs/wires) from a netlist.
* Returns the index of the port if found, else -1.
*/
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

// Function to write the module instantiation in Verilog
std::string module_instantiation(Graph HLSM)
{
	std::vector<Data> inputs = HLSM.get_inputs();
	std::vector<Data> outputs = HLSM.get_outputs();
	std::vector<Data> reg_variables = HLSM.get_variables();

	// Instantiate HLSM module per assignment requirements
	std::string module_def = std::string("module HLSM (Clk, Rst, Start, Done, ");

	for (unsigned int a = 0; a < inputs.size(); a++)
	{
		module_def += inputs[a].get_name() + std::string(", ");
	}

	for (unsigned int b = 0; b < outputs.size(); b++)
	{
		module_def += outputs[b].get_name();
		module_def += (b == outputs.size() - 1) ? (std::string(";\n ")) : (std::string(", "));
	}
	
	// Define input ports
	std::string input_ports = "\t" + std::string("input Clk, Rst, Start;") + "\n";
	input_ports += "\t" + std::string("input ");

	for (unsigned int i = 0; i < inputs.size(); i++)
	{
		// Append each new input to the input_ports string
		input_ports += inputs[i].get_name();

		
		// If at the end of the input list, then append a semicolon; else append a comma
		input_ports += (i == inputs.size()) ? std::string(";\n") : std::string(", ");
	}

	std::string output_ports = "\t" + std::string("output reg Done;") + "\n";

	for (unsigned int j = 0; j < outputs.size(); j++)
	{
		// Append each new output to the output_ports string
		output_ports += outputs[j].get_name();

		// If at the end of the output list, then append a semicolon; else append a comma
		output_ports += (j == outputs.size()) ? std::string(";\n") : std::string(", ");
	}

	// Define all local variables as reg variables
	std::string variable_ports = "\t" + std::string("variable reg ");

	for (unsigned int k = 0; k < reg_variables.size(); k++)
	{
		// Append each new variable to the reg_variables string
		variable_ports += reg_variables[k].get_name();

		// If at the end of the reg_variables vector, then append a semicolon; else append a comma
		variable_ports += (k == reg_variables.size()) ? std::string(";\n") : std::string(", ");
	}

	// Define the state register
	std::string state_register = "\t" + std::string("reg [") + std::string(":0] State;\n");

	std::string HLSM_module = module_def + input_ports + output_ports + variable_ports + state_register;

	return HLSM_module;

}

// Function to write Verilog code when Rst == 1
std::string comb_logic_reset(Graph HLSM)
{
	std::vector<Data> reg_variables = HLSM.get_variables();
	std::vector<Data> outputs = HLSM.get_outputs();

	std::string reset_logic = "\n\t" + std::string("always @ (posedge Clk) begin") + "\n";

	// Logic for when Rst is set to 1
	reset_logic += "\t\t" + std::string("if (Rst == 1) begin") + "\n";

	// Use a for loop to set all variables equal to 0 and the State to Wait, per the Rst == 1
	for (unsigned int i = 0; i <= reg_variables.size(); i++)
	{
		// Three indents for resetting all variables to 0
		reset_logic += "\t\t\t";

		if (i < reg_variables.size())
		{
			reset_logic += reg_variables[i].get_name() + std::string(" <= 0;") + "\n";
		}

	}

	for (unsigned int j = 0; j <= outputs.size(); j++)
	{
		reset_logic += "\t\t\t";

		if (j < outputs.size())
		{
			reset_logic += outputs[j].get_name() + std::string(" <= 0;") + "\n";
		}

		else
		{
			reset_logic += "Done <= 0;\n";				// Set Done = 0
			reset_logic += "\t\t\tState <= Wait;\n";   // Reset the HLSM back to the Wait state
		}

	}

	reset_logic += "\t\t" + std::string("end\n");

	return reset_logic;

}

// Function to write state Logic
std::string write_state_logic(unsigned int j, std::vector<Operation> time_index_ops)
{
	std::string state = "\t\t\t\t" + std::string("State") + std::to_string(j) + std::string(" : begin") + "\n";

	// Iterate through all the scheduled nodes for the time index
	for (unsigned int i = 0; i < time_index_ops.size(); i++)
	{
		std::string node_instantiation = "\t\t\t\t\t";

		std::string bracketed_equals = "<=";
		std::string line = time_index_ops[i].get_line() + "\n";

		line.replace(2, 1, bracketed_equals);

		state += node_instantiation;
	}

	// Write transition to next state
	state += "\t\t\t\t\t" + std::string("State <= State") + std::to_string(j+1) + "\n";

	return state;
}

// Function to write Verilog code when Rst is not pressed
std::string comb_logic_else(Graph HLSM)
{

	std::vector<std::vector<Operation>> index_scheduled_operations = HLSM.get_nodes_ordered_by_time();

	std::string else_logic = "\t\telse begin\n" + std::string("\t\t\tDone <= 0;\n");

	else_logic += "\t\t\t" + std::string("case (State)") + "\n";

	// Wait State Logic
	else_logic += "\t\t\t\t" + std::string("Wait : begin") + "\n"
		+ "\t\t\t\t\t" + std::string("if (Start == 1) begin") + "\n"
		+ "\t\t\t\t\t\t" + std::string("State <= State1") + "\n"
		+ "\t\t\t\t\t" + std::string("end") + "\n";

	// Vector of vectors structure: a vector where each nested vector represents the operations in one time frame
	for (unsigned int i = 0; i < index_scheduled_operations.size(); i++)
	{
		else_logic += write_state_logic(i, index_scheduled_operations[i]);
	}

	// Final State Logic
	else_logic += "\t\t\t\t" + std::string("Final : begin") + "\n"
		+ "\t\t\t\t\t" + std::string("State <= Wait") + "\n"
		+ "\t\t\t\t" + std::string("end") + "\n";

	else_logic += "\t\t\t" + std::string("endcase") + "\n";
	else_logic += "\t\t" + std::string("end") + "\n";
	else_logic += "\t" + std::string("end") + "\n";
	else_logic += std::string("endmodule");

	return else_logic;
}

std::string write_Verilog_code(Graph state_machine)
{
	std::string verilog_file = module_instantiation(state_machine) + comb_logic_reset(state_machine) + comb_logic_else(state_machine);

	return verilog_file;
}