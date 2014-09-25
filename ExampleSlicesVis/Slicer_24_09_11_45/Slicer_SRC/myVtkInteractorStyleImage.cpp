#include "myVtkInteractorStyleImage.h"
#include <vtkRendererCollection.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include "constants.h"
#include <vtkDataSetMapper.h>
#include <vtkStructuredPoints.h>
#include <vtkCell.h>
#include <vtkPointData.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkExtractVOI.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkImageMapToColors.h>
#include <vtkPropPicker.h>


myVtkInteractorStyleImage::myVtkInteractorStyleImage()
{
	leapCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	leapCallback->SetCallback(myVtkInteractorStyleImage::ProcessLeapEvents);
}

void myVtkInteractorStyleImage::SetImageViewer(vtkImageViewer2* imageViewer, int size_x, int size_y, vtkSmartPointer<vtkStructuredPoints> selection, vtkSmartPointer<vtkImageActor> selection_actor) {
	_ImageViewer = imageViewer;
	_window_size_x = size_x;
	_window_size_y = size_y;
	_orientation   = imageViewer->GetSliceOrientation();
	_MinSlice	   = imageViewer->GetSliceMin();
	_MaxSlice      = imageViewer->GetSliceMax();
	_drawSize= DEFAULT_DRAW_SIZE;
	leapCallback->SetClientData(this);
	_Slice = _MinSlice;
	_isSliceLocked = false;
	_isPainting = false;
	_selection = vtkSmartPointer<vtkImageMapToColors>::New();
	_3D_selection = selection;
	_selection->SetInputData(selection);
	_selection_actor = selection_actor;
	cout << "Slicer: Min = " << _MinSlice << ", Max = " << _MaxSlice << ", Orientation: " << _orientation << std::endl;
}

void myVtkInteractorStyleImage::SetStatusMapper(vtkTextMapper* statusMapper) {
	_StatusMapper = statusMapper;
}

void myVtkInteractorStyleImage::setSlice(int slice){
	if(this->Interactor->GetShiftKey()){
		this->_Slice = slice;
	}
}

void myVtkInteractorStyleImage::startPainting(bool state) {
	//todo: fill it?
}

int myVtkInteractorStyleImage::getMaxSlice(){
	return this->_MaxSlice;
}

void myVtkInteractorStyleImage::MoveSliceForward() {
	if(_Slice < _MaxSlice) {
		_Slice += 1;
		cout << "MoveSliceForward::Slice = " << _Slice << std::endl;
		_ImageViewer->SetSlice(_Slice);
		int displayExtent[6];
		_ImageViewer->GetImageActor()->GetDisplayExtent(displayExtent);
		_selection_actor->SetDisplayExtent(displayExtent);
		//std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
		//_StatusMapper->SetInput(msg.c_str());
		_ImageViewer->Render();
	}
}

void myVtkInteractorStyleImage::MoveSliceBackward() {
	if(_Slice > _MinSlice) {
		_Slice -= 1;
		cout << "MoveSliceBackward::Slice = " << _Slice << std::endl;
		_ImageViewer->SetSlice(_Slice);
		int displayExtent[6];
		_ImageViewer->GetImageActor()->GetDisplayExtent(displayExtent);
		_selection_actor->SetDisplayExtent(displayExtent);
		//std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
		//_StatusMapper->SetInput(msg.c_str());
		_ImageViewer->Render();
	}
}

void myVtkInteractorStyleImage::ToggleOrientation() {
	_orientation = (_orientation+1)%3;
	switch(_orientation){
	case 0:
		_ImageViewer->SetSliceOrientationToXY();
		break;
	case 1:
		_ImageViewer->SetSliceOrientationToXZ();
		break;
	case 2:
		_ImageViewer->SetSliceOrientationToYZ();
		break;
	}
	cout << "Slice orientation: " << _orientation << endl;
	_ImageViewer->SetSlice(0);
	_ImageViewer->Render();
}

void myVtkInteractorStyleImage::OnKeyDown() {
	std::string key = this->GetInteractor()->GetKeySym();
	if(key.compare("Up") == 0) {
		cout << "Up arrow key was pressed." << endl;
		MoveSliceForward();
	}
	else if(key.compare("Down") == 0) {
		cout << "Down arrow key was pressed." << endl;
		MoveSliceBackward();
	}else if(key.compare("1") == 0) {
		cout << "Draw size was changed to " <<_drawSize-1<< endl;
		this->_drawSize=std::max(MIN_DRAW_SIZE,_drawSize-1);
	}else if(key.compare("2") == 0) {
		cout << "Draw size was changed to " <<_drawSize+1 << endl;
		this->_drawSize=std::min(MAX_DRAW_SIZE,_drawSize+1);
	}
	else if(key.compare("o") == 0) {
		cout << "Orientation key was pressed." << endl;
		ToggleOrientation();
	}
	else if(key.compare("shift") == 0) {
		cout << "Shift key was pressed." << endl;
		lockSlice(false);
	}
	else if (key.compare("Ctrl") == 0) {
		cout << "control key was pressed. Doing something..." << endl;
	}
	// forward event
}

void myVtkInteractorStyleImage::OnKeyUp() {
	std::string key = this->GetInteractor()->GetKeySym();
	if(key.compare("shift") == 0) {
		cout << "shift key was released." << endl;
		lockSlice(true);
	}
	// forward event
}


void myVtkInteractorStyleImage::OnMouseWheelForward() {
	//std::cout << "Scrolled mouse wheel forward." << std::endl;
	MoveSliceForward();
	// don't forward events, otherwise the image will be zoomed 
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	// vtkInteractorStyleImage::OnMouseWheelForward();
}


void myVtkInteractorStyleImage::OnMouseWheelBackward() {
	//std::cout << "Scrolled mouse wheel backward." << std::endl;
	if(_Slice > _MinSlice) {
		MoveSliceBackward();
	}
	// don't forward events, otherwise the image will be zoomed 
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	// vtkInteractorStyleImage::OnMouseWheelBackward();
}

void myVtkInteractorStyleImage::lockSlice(bool state){
	_isSliceLocked = state;
}

void myVtkInteractorStyleImage::SetInteractor(vtkRenderWindowInteractor *i)
{
	if(i == this->Interactor)
	{
		return;
	}
	// if we already have an Interactor then stop observing it
	if(this->Interactor)
	{
		this->Interactor->RemoveObserver(this->EventCallbackCommand);
	}
	this->Interactor = i;
	// add observers for each of the events handled in ProcessEvents
	if(i)
	{
		i->AddObserver(vtkCommand::EnterEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::LeaveEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::MouseMoveEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::LeftButtonPressEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::LeftButtonReleaseEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::MiddleButtonPressEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::MiddleButtonReleaseEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::RightButtonPressEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::RightButtonReleaseEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::MouseWheelForwardEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::MouseWheelBackwardEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::ExposeEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::ConfigureEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::TimerEvent,
			this->leapCallback,
			this->Priority);
		i->AddObserver(vtkCommand::KeyPressEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::KeyReleaseEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::CharEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::DeleteEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::TDxMotionEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::TDxButtonPressEvent,
			this->EventCallbackCommand,
			this->Priority);
		i->AddObserver(vtkCommand::TDxButtonReleaseEvent,
			this->EventCallbackCommand,
			this->Priority);
	}
	this->EventForwarder->SetTarget(this->Interactor);
	if (this->Interactor)
	{
		this->AddObserver(vtkCommand::StartInteractionEvent, this->EventForwarder);
		this->AddObserver(vtkCommand::EndInteractionEvent, this->EventForwarder);
	}
	else
	{
		this->RemoveObserver(this->EventForwarder);
	}
}

void myVtkInteractorStyleImage::ProcessLeapEvents(vtkObject* object, unsigned long event, void* clientdata, void* callData){
	//cout << "Got a leap event!"<< endl;
	vtkSmartPointer<myVtkInteractorStyleImage> intStyle = 
		reinterpret_cast<myVtkInteractorStyleImage*>(clientdata);
	vtkRendererCollection* rc = intStyle->_ImageViewer->GetRenderWindow()->GetRenderers();
	rc->InitTraversal(); // initialize the traversal to start from 0
	vtkActor* cross_actor;
	vtkActor* selection_actor;
	for (int i = 0; i < rc->GetNumberOfItems(); i++){
		if (i == SELECTION_LAYER) {
			vtkActorCollection* col = rc->GetNextItem()->GetActors();
			cross_actor = col->GetLastActor();
			selection_actor = col->GetLastActor();
			//cout << "selection actor" <<selection_actor << endl;
			continue;
		}// else if (i == CROSS_LAYER){
		//	cross_actor = rc->GetNextItem()->GetActors()->GetLastActor();
		//	break;
		//}
		rc->GetNextItem()->GetActors()->GetNumberOfItems();
	}
	//cout << "searching for religion:"<<cross_actor<<endl;
	vtkSmartPointer<vtkPolyData> pd = (vtkPolyData *)((vtkPolyDataMapper*)(cross_actor->GetMapper())->GetInputAsDataSet());
	//cout << "almost found jeezus" <<endl;
	vtkSmartPointer<vtkImageMapToColors> selection_mapper = (vtkImageMapToColors*)intStyle->_selection_actor->GetMapper()->GetInputAlgorithm();
	//cout << "found jeezus" <<endl;
	vtkSmartPointer<vtkStructuredPoints> selection_structured_points = (vtkStructuredPoints *)selection_mapper->GetInput();

	double* size = selection_structured_points->GetBounds();

	// changing the crosshair
	vtkSmartPointer<vtkPoints> new_pts =
		vtkSmartPointer<vtkPoints>::New();
	double cross_x = std::max(size[0],std::min(SCALE_FACTOR*intStyle->_x_position,size[1]));
	double cross_y = std::max(size[2],std::min(SCALE_FACTOR*(intStyle->_y_position+150),size[3]));
	new_pts->InsertNextPoint(size[0], cross_y, size[5]);
	new_pts->InsertNextPoint(size[1], cross_y, size[5]);
	new_pts->InsertNextPoint(cross_x, size[2], size[5]);
	new_pts->InsertNextPoint(cross_x, size[3], size[5]);
	pd->SetPoints(new_pts);

	// When SHIFT key is pressed, udpate slice
	if (intStyle->Interactor->GetShiftKey()){
		cout << "Entered Shift" << endl;
		intStyle->_ImageViewer->SetSlice(intStyle->_Slice);
		int displayExtent[6];
		intStyle->_ImageViewer->GetImageActor()->GetDisplayExtent(displayExtent);
		intStyle->_selection_actor->SetDisplayExtent(displayExtent);
		cout << "Exit shift." << endl;
	}

	int *selExt = selection_structured_points->GetExtent();

	// check whether to start drawing on screen.
	if (intStyle->Interactor->GetControlKey()) {
		cout << "ctrl pressed" <<endl;
		vtkPointData* cellData = selection_structured_points->GetPointData();
		vtkIntArray* selection_scalars = (vtkIntArray*) cellData->GetScalars();
		double x[3]={cross_x,cross_y,size[5]};
		int ijk[3];
		double pCoord[3];
		//selection_structured_points->GetCellBounds(cellId,bounds);
		selection_structured_points->ComputeStructuredCoordinates(x,ijk,pCoord);
		cout << " cell IJK is: "<<ijk[0]<<":"<<ijk[1] <<":"<<intStyle->_Slice <<endl;
		ijk[2]=intStyle->_Slice;
		int ijk2[3]={0,0,ijk[2]};
		int minX = std::max(0,ijk[0]-intStyle->_drawSize);
		int maxX = std::min(ijk[0]+intStyle->_drawSize,selExt[1]);
		int minY = std::max(ijk[1]-intStyle->_drawSize,0);
		int maxY = std::min(ijk[1]+intStyle->_drawSize,selExt[3]);
		for(int i=minX;i<maxX;i++){
			ijk2[0]=i;
			for(int j=minY;j<maxY;j++){
				ijk2[1]=j;
				vtkIdType cellId = selection_structured_points->ComputePointId(ijk2);
				selection_scalars->SetValue(cellId,ACTIVE);
			}
		}

		//Update the underlying data object.
		selection_scalars->Modified();

		// change the crosshair's color
		vtkUnsignedCharArray* da = (vtkUnsignedCharArray*)pd->GetCellData()->GetScalars();
		da->SetValue(0, 255);
		da->SetValue(1, 0);
		da->SetValue(2, 0);
		da->SetValue(3, 255);
		da->SetValue(4, 0);
		da->SetValue(5, 0);
	}else if(intStyle->Interactor->GetAltKey()){
		cout << "ctrl pressed" <<endl;
		vtkPointData* cellData = selection_structured_points->GetPointData();
		vtkIntArray* selection_scalars = (vtkIntArray*) cellData->GetScalars();
		double x[3]={cross_x,cross_y,size[5]};
		int ijk[3];
		double pCoord[3];
		selection_structured_points->ComputeStructuredCoordinates(x,ijk,pCoord);
		cout << " cell IJK is: "<<ijk[0]<<":"<<ijk[1] <<":"<<intStyle->_Slice <<endl;
		ijk[2]=intStyle->_Slice;
		int ijk2[3]={0,0,ijk[2]};
		int minX = std::max(0,ijk[0]-intStyle->_drawSize);
		int maxX = std::min(ijk[0]+intStyle->_drawSize,selExt[1]);
		int minY = std::max(ijk[1]-intStyle->_drawSize,0);
		int maxY = std::min(ijk[1]+intStyle->_drawSize,selExt[3]);
		for(int i=minX;i<maxX;i++){
			ijk2[0]=i;
			for(int j=minY;j<maxY;j++){
				ijk2[1]=j;
				vtkIdType cellId = selection_structured_points->ComputePointId(ijk2);
				selection_scalars->SetValue(cellId,NOT_ACTIVE);
			}
		}

		//Update the underlying data object.
		selection_scalars->Modified();

		// change the crosshair's color
		vtkUnsignedCharArray* da = (vtkUnsignedCharArray*)pd->GetCellData()->GetScalars();
		da->SetValue(0, 0);
		da->SetValue(1, 255);
		da->SetValue(2, 0);
		da->SetValue(3, 0);
		da->SetValue(4, 255);
		da->SetValue(5, 0);
	}else {

		// change the crosshair's color
		vtkUnsignedCharArray* da2 = (vtkUnsignedCharArray*)pd->GetCellData()->GetScalars();
		da2->SetValue(0, 0);
		da2->SetValue(1, 0);
		da2->SetValue(2, 255);
		da2->SetValue(3, 0);
		da2->SetValue(4, 0);
		da2->SetValue(5, 255);

	}

	// render
	cross_actor->GetMapper()->Update();
	intStyle->_selection_actor->GetMapper()->Update();

	//intStyle->_ImageViewer->SetSlice(intStyle->_Slice);
	intStyle->_ImageViewer->Render();
}

vtkStandardNewMacro(myVtkInteractorStyleImage);