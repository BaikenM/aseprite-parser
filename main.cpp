#include "Aseprite_parser.h"

using namespace aseprite;

int main(int argc, char* argv[])
{
	std::cout << "Running aseprite parser tests..." << std::endl;
	
	try
	{
		Aseprite sprite{ "../../../test.aseprite" };
		std::cout << "TEST SUCCESSFUL." << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "TEST FAILED." << std::endl;
		std::cerr << e.what() << std::endl;
	}
	return 0;
}