#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "application/Application.h"

int main()
{
	Application app;
	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cout << "failure\n";
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}