#include <iostream>
#include "raspicaminterface.h"


int main() {
std::cout << "hello" << std::endl;

CameraHandle cam = raspicam_create_camera();
raspicam_set_dims(cam, 640,480);
raspicam_open(cam);
raspicam_grab(cam);
unsigned char *data = new unsigned char[ 640*480*3 ];
raspicam_retrieve(cam, data);
raspicam_release(cam);
raspicam_destroy_camera(cam);
delete[] data;

return 0;
}

