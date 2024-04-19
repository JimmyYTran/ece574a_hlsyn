#include "Data.h"

Data::Data()
{
	this->name = "";
	this->datatype = "";
	this->datawidth = 0;
	this->is_signed = false;
}

Data::Data(std::string name, std::string datatype, int datawidth, bool is_signed)
{
	this->name = name;
	this->datatype = datatype;
	this->datawidth = datawidth;
	this->is_signed = is_signed;
}