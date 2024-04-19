#include "Operation.h"

Operation::Operation()
{
	this->name = "";
}

Operation::Operation(std::string name)
{
	this->name = name;
}

void Operation::add_input(Data input)
{
	this->inputs.push_back(input);
}