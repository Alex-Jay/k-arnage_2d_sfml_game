#include "Application.hpp"

#include <iostream>

//#include "MapTiler.hpp"

int main()
{
	try
	{
		Application app;
		app.run();
	}
	catch (std::exception e)
	{
		std::cout << "\n EXCEPTION: " << e.what() << std::endl;
	}
}
