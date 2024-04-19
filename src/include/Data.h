#ifndef DATA_H
#define DATA_H

#include <string>

class Data
{
private:
	std::string name;
	std::string datatype;
	int datawidth;
	bool is_signed;
public:
	Data();
	Data(std::string name, std::string datatype, int datawidth, bool is_signed);

	std::string get_name() { return name; }
	std::string get_datatype() { return datatype; }
	int get_datawidth() { return datawidth; }
	bool get_is_signed() { return is_signed; }
};

#endif