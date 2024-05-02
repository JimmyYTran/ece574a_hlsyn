#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Operation.h"
#include "graph.h"

static const std::vector<std::string> OPERATION_SYMBOLS =
std::vector<std::string>{ "*", ">", "<", "==", "?", ">>", "<<", "/", "%" };

static const std::vector<std::string> MODULES =
std::vector<std::string>{ "MUL", "COMP>", "COMP<", "COMP==", "MUX2x1", "SHR", "SHL", "DIV", "MOD" };

std::vector<std::string> parse_netlist_lines
(std::vector<std::string> lines, std::string filename, std::vector<Operation>& operations);

std::vector<std::string> parse_line
(std::vector<std::string> lines, int current_line, std::vector<Data> ports, std::vector<Operation>& operations, int& num_processed_lines);

std::string parse_if_statement
(std::vector<std::string> lines, int line_index, std::vector<Data> ports, std::vector<Operation>& operations, int& lines_processed);

std::string strip_comment_from_line(std::string line);

std::vector<std::string> remove_empty_lines(std::vector<std::string> lines);

std::string write_module_definition(std::vector<Data> ports, std::string circuit_name);

std::string create_port_declaration_from_line(std::string line, std::vector<Data>& ports);

std::string create_module_instance_from_line
(std::string line, int line_num, std::vector<Data> ports, std::vector<Operation>& operations);

std::string determine_module(std::vector<std::string>);

Operation parse_line_to_operation
(std::vector<std::string> split_line, std::string module_type, std::vector<Data>& ports);

std::string write_input_list(Operation operation);

std::vector<std::string> split_string(std::string line);

int find_port(std::vector<Data> ports, std::string new_port_name);

std::string module_instantiation(Graph HLSM);

std::string comb_logic_reset(Graph HLSM);

std::string write_state_logic(unsigned int j, std::vector<Operation> time_index_ops);

std::string comb_logic_else(Graph HLSM);

std::string write_Verilog_code(Graph state_machine);


#endif