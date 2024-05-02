#ifndef WRITEVERILOG_H
#define WRITEVERILOG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cmath>

#include "Operation.h"
#include "graph.h"

std::string module_instantiation(Graph HLSM);

std::string comb_logic_reset(Graph HLSM);

std::string write_scheduled_state(std::vector<Operation> scheduled_ops, unsigned int j);

std::string write_if_statement(Operation op, unsigned int indent);

std::string write_normal_statement(Operation ops, unsigned int indent);

std::string comb_logic_else(Graph HLSM);

std::string write_Verilog_code(Graph state_machine);


#endif