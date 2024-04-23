#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Operation.h"

static const std::vector<std::string> OPERATION_SYMBOLS =
std::vector<std::string>{ "*", ">", "<", "==", "?", ">>", "<<", "/", "%" };

static const std::vector<std::string> MODULES =
std::vector<std::string>{ "MUL", "COMP>", "COMP<", "COMP==", "MUX2x1", "SHR", "SHL", "DIV", "MOD" };

std::vector<std::string> parse_netlist_lines
(std::vector<std::string> lines, std::string filename, std::vector<Operation>& operations);

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

#endif