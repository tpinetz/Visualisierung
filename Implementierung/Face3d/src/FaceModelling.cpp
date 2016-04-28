#include <iostream>
#include "FaceCoordinates3d.hpp"
#include "Viewer.hpp"



int main(int argc, char** argv)
{

	try
	{
		Face3D::FaceCoordinates3d faceCoords;
		faceCoords.fromFile("ipc/faceGeometry.txt");
		// TODO: 3d coordinates are in variable faceCoords
		// do something with those coordinates: reshape model such that it fits to those coordinates

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

