#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <tuple>

#include "Operation.h"

static const std::vector<std::string> OPERATION_SYMBOLS =
std::vector<std::string>{ "*", ">", "<", "==", "?", ">>", "<<", "/", "%" };

static const std::vector<std::string> MODULES =
std::vector<std::string>{ "MUL", "COMP>", "COMP<", "COMP==", "MUX2x1", "SHR", "SHL", "DIV", "MOD" };

struct Ports {
    std::vector<Data> inputs;
    std::vector<Data> outputs;
    std::vector<Data> variables;
    int name_in_inputs(std::string name) {
        for (int i = 0; i < inputs.size(); i++) {
            if (inputs[i].get_name().compare(name) == 0) {
                return i;
            }
        }
        return -1;
    }
    int name_in_outputs(std::string name) {
        for (int i = 0; i < outputs.size(); i++) {
            if (outputs[i].get_name().compare(name) == 0) {
                return i;
            }
        }
        return -1;
    }
    int name_in_variables(std::string name) {
        for (int i = 0; i < variables.size(); i++) {
            if (variables[i].get_name().compare(name) == 0) {
                return i;
            }
        }
        return -1;
    }
    bool name_in_port(std::string name) {
        name_in_inputs(name) != -1 || name_in_outputs(name) != -1 || name_in_variables(name) != -1;
    }
    std::optional<Data> get_data_with_name(std::string name) {
        int i = name_in_inputs(name);
        int o = name_in_outputs(name);
        int v = name_in_variables(name);
        if (i != -1) {
            return inputs[i];
        }
        if (o != -1) {
            return outputs[o];
        }
        if (v != -1) {
            return variables[v];
        }
        return {};
    }
    std::vector<Data> get_all_ports() {
        std::vector<Data> p;
        for (auto a : inputs) {
            p.push_back(a);
        }
        for (auto a : outputs) {
            p.push_back(a);
        }
        for (auto a : variables) {
            p.push_back(a);
        }
        return p;
    }
};

std::tuple<std::vector<Operation>, Ports, std::string> parse(std::vector<std::string> lines);
std::tuple<std::vector<Operation>, std::string> parse_main(std::vector<std::string> lines, Ports ports, int& parsed_lines);
std::tuple<Operation, std::string> parse_if_statement(std::vector<std::string>, Ports ports, int& parsed_lines);
std::tuple<Operation, std::string> parse_single_line(std::vector<std::string>, Ports ports, int& parsed_lines);
std::tuple<Ports, std::string> parse_ports(std::vector<std::string> lines, int& parsed_lines);
std::string strip_comment_from_line(std::string line);
std::vector<std::string> cleanup_lines(std::vector<std::string> lines);
std::vector<std::string> split_string(std::string line);
std::string determine_module(std::vector<std::string> split_line);
Operation parse_line_to_operation
(std::vector<std::string> split_line, std::string module_type, std::vector<Data> ports);
int find_port(std::vector<Data> ports, std::string new_port_name);


#endif