#include "writeVerilog.h"


#include "Operation.h"
#include "graph.h"

// Function to write the module instantiation in Verilog
std::string module_instantiation(Graph HLSM)
{
	std::vector<Data> inputs = HLSM.get_inputs();
	std::vector<Data> outputs = HLSM.get_outputs();
	std::vector<Data> reg_variables = HLSM.get_variables();

	// Instantiate HLSM module per assignment requirements
	std::string module_def = std::string("module HLSM (Clk, Rst, Start, Done, ");

	// Iterate through graph's inputs, append the name to the module_def string and end with comma
	for (unsigned int a = 0; a < inputs.size(); a++)
	{
		module_def += inputs[a].get_name() + std::string(", ");
	}

	// Iterate through graph's outputs, append the name to the module_def string and end with );
	for (unsigned int b = 0; b < outputs.size(); b++)
	{
		module_def += outputs[b].get_name();
		module_def += (b == outputs.size() - 1) ? (std::string(");\n ")) : (std::string(", "));
	}

	// Define input ports
	std::string input_ports = "\t" + std::string("input Clk, Rst, Start;") + "\n";
	input_ports += "\t" + std::string("input ");

	for (unsigned int i = 0; i < inputs.size(); i++)
	{
		// Append each new input to the input_ports string
		input_ports += inputs[i].get_name();

		// If at the end of the input list, then append a semicolon; else append a comma
		input_ports += (i == inputs.size() - 1) ? std::string(";\n") : std::string(", ");
	}

	std::string output_ports = "\t" + std::string("output reg Done;") + "\n";
	output_ports += "\t" + std::string("output ");

	for (unsigned int j = 0; j < outputs.size(); j++)
	{
		// Append each new output to the output_ports string
		output_ports += outputs[j].get_name();

		// If at the end of the output list, then append a semicolon; else append a comma
		output_ports += (j == outputs.size() - 1) ? std::string(";\n") : std::string(", ");
	}

	// Define all local variables as reg variables
	std::string variable_ports = "\t" + std::string("variable reg ");

	for (unsigned int k = 0; k < reg_variables.size(); k++)
	{
		// Append each new variable to the reg_variables string
		variable_ports += reg_variables[k].get_name();

		// If at the end of the reg_variables vector, then append a semicolon; else append a comma
		variable_ports += (k == reg_variables.size() - 1) ? std::string(";\n") : std::string(", ");
	}

	// Define the state register
	std::string state_register = "\t" + std::string("reg [") + std::to_string((int)(ceil(log2(HLSM.get_latency_constraint())))) + std::string(":0] State;\n");

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
	for (unsigned int i = 0; i < reg_variables.size(); i++)
	{
		// Three indents for resetting all variables to 0
		reset_logic += "\t\t\t";

		if (i < reg_variables.size())
		{
			reset_logic += reg_variables[i].get_name() + std::string(" <= 0;") + "\n";
		}

	}

	for (unsigned int j = 0; j < outputs.size(); j++)
	{
		reset_logic += "\t\t\t";
		reset_logic += outputs[j].get_name() + std::string(" <= 0;") + "\n";
	}

	reset_logic += "\t\t\tDone <= 0;\n";				// Set Done = 0
	reset_logic += "\t\t\tState <= Wait;\n";   // Reset the HLSM back to the Wait state

	reset_logic += "\t\t" + std::string("end\n");

	return reset_logic;

}

std::string write_scheduled_state(std::vector<Operation> scheduled_ops, unsigned int j)
{
	std::string state_ops = "\t\t\t\t" + std::string("State") + std::to_string(j) + std::string(": begin\n");

	for (unsigned int i = 0; i < scheduled_ops.size(); i++)
	{
		
		if (scheduled_ops[i].get_name().compare("IF") == 0) {
			state_ops += write_if_statement(scheduled_ops[i], 5);
		}
		else
		{
			state_ops += write_normal_statement(scheduled_ops[i], 5);
		}
	}

	state_ops += std::string("\t\t\t\t\tState <= State") + std::to_string(j+1) + std::string(";\n");
	state_ops += "\t\t\t\t" + std::string("end\n");

	return state_ops;
}

std::string write_if_statement(Operation op, unsigned int indent)
{

	std::string if_statement = {};
	for (unsigned int i = 0; i < indent; i++)
	{
		if_statement += "\t";
	}

	if_statement += std::string("if (") + op.if_condition.value().get_name() + std::string(") begin\n");

	for (Operation o : op.if_body)
	{

		if (o.get_name().compare("IF") == 0) {
			if_statement += write_if_statement(o, indent + 1);
		}
		else
		{
			if_statement += write_normal_statement(o, indent);
		}
	}

	for (unsigned int i = 0; i < indent; i++)
	{
		if_statement += "\t";
	}

	if_statement += std::string("end\n");
	
	for (unsigned int i = 0; i < indent; i++)
	{
		if_statement += "\t";
	}

	if_statement += std::string("else begin\n");

	for (Operation o : op.else_body)
	{
		if (o.get_name().compare("IF") == 0) {
			if_statement += write_if_statement(o, indent + 1);
		}
		else
		{
			if_statement += write_normal_statement(o, indent);
		}
	}

	for (unsigned int i = 0; i < indent; i++)
	{
		if_statement += "\t";
	}

	if_statement += std::string("end\n");

	return if_statement;

}

// Function to write normal state logic
std::string write_normal_statement(Operation ops, unsigned int indent)
{
	std::string line = {};
	for (unsigned int i = 0; i < indent; i++)
	{
		line += "\t";
	}
	std::string node = ops.get_line() + ";\n";

	size_t pos = node.find('=');
	while (pos != std::string::npos) {
		node.replace(pos, 1, "<=");
		pos = node.find('=', pos + 2); // Start searching from the next position
	}

	line += node;

	return line;
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
		+ "\t\t\t\t\t\t" + std::string("State <= State0") + "\n"
		+ "\t\t\t\t\t" + std::string("end") + "\n";

	// Vector of vectors structure: a vector where each nested vector represents the operations in one time frame
	for (unsigned int i = 0; i < index_scheduled_operations.size(); i++)
	{
		// Call write_scheduled_state to write the operations in a time index aka Statei
		else_logic += write_scheduled_state(index_scheduled_operations[i], i);
	}

	// Final State Logic
	else_logic += "\t\t\t\t" + std::string("Final : begin") + "\n"
		+ "\t\t\t\t\t" + std::string("Done <= 1;") + "\n"
		+ "\t\t\t\t\t" + std::string("State <= Wait;") + "\n"
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
