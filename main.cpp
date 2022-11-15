#include "Aseprite_parser.h"

using namespace std;
using namespace aseprite;

int main(int argc, char* argv[])
{
	cout << "Running aseprite parser tests..." << endl;
	
	try
	{
		Aseprite sprite{ "../../../test.aseprite" };
		
		sprite.fileInfo();
		cout << "TEST SUCCESSFUL." << endl;
	}
	catch (const std::exception& e)
	{
		cerr << "TEST FAILED." << endl;
		cerr << e.what() << endl;
	}
	return 0;
}
