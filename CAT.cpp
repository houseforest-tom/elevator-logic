//============================================================================
// Name        : CAT.cpp
// Author      : Jonas Rinast
// Version     :
// Copyright   : Copyright by Jonas Rinast, Institute for Software Systems, Hamburg University of Technology
// Description : Compilation and Testing (CAT) C++ Framework
//============================================================================

#include <iostream>
#include <vector>

#include "ElevatorEnvironment.h"

int main(int argc, char **argv) {

	std::string config = DEFAULT_TESTFILE;

	if (argc >= 2) {
		config.assign(argv[1]);
	}

	try {

		ElevatorEnvironment env;

		std::cout << "Running test: " << config << std::endl;

		env.Load(config);
		env.Run();
		env.Evaluate();

	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	std::cout << "Test " << config << " ran successfully." << std::endl;

	return 0;
}
