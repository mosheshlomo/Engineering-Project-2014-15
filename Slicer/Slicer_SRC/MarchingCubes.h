#ifndef __MARCHING_CUBES__
#define __MARCHING_CUBES__

#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkMarchingCubes.h>
#include <vtkVoxelModeller.h>
#include <vtkSphereSource.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include <vtkStructuredPoints.h>
#include "constants.h"
#include <vtkImageViewer2.h>
#include <vtkImageMaskBits.h>
#include <vtkCamera.h>



class MarchingCubes {
	
public:
	MarchingCubes(vtkStructuredPoints* selection);
	~MarchingCubes();
	
	void render3D();
	void flipView(bool flip);
private:
	vtkSmartPointer<vtkMarchingCubes>  _surface;
	vtkSmartPointer<vtkRenderer>       _renderer;
	vtkSmartPointer<vtkRenderWindow>   _renderWindow;
	vtkSmartPointer<vtkRenderWindowInteractor> _renderWindowInteractor;
	vtkSmartPointer<vtkPolyDataMapper> _mapper;
	vtkSmartPointer<vtkActor>          _actor;
	vtkSmartPointer<vtkImageMaskBits>  _mask;

	vtkSmartPointer<vtkImageViewer2>   _viewer;

};




#endif