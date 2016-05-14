#include <iostream>
#include "FaceCoordinates3d.hpp"
#include "Viewer.hpp"



int main(int argc, char** argv)
{

	try
	{		
		Face3D::Viewer viewer;
		viewer.initOpenGL();
		viewer.run();
	}
	catch (std::exception e)
	{
		std::cout<<"Exception: "<<e.what()<<"\n";
		getchar();
	}
	catch (...)
	{
		std::cout << "Exception\n";
		getchar();
	}
	

	return 0;
}

