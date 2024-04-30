#include "fileio.h"
#include "graph.h"

int main(int argc, char* argv[])
{
	Graph HLSM = Graph();
	Operation node1 = Operation("node1", "t1 = a + b");
	node1.set_fds_time(1);
	HLSM.add_node(node1);
	
	return 0;
}