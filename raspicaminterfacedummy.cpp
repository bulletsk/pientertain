#include "raspicaminterfacedummy.h"
#include <string>
#include <iostream>

CameraHandle raspicam_create_camera()
{
  unsigned char *ret = new unsigned char[640*480*3];
  for (int y=0;y<480;y++) {
    for (int x=0;x<640;x++) {
      ret[ 3*(y*640+x)+0 ] = (x%255);
      ret[ 3*(y*640+x)+1 ] = (y%255);
      ret[ 3*(y*640+x)+2 ] = ((x+y)%255);
    }
  }

  return (CameraHandle)ret;
}
bool raspicam_destroy_camera(CameraHandle handle)
{
  unsigned char *cam = (unsigned char *)handle;
  delete cam;
  return true;
}

bool raspicam_open(CameraHandle)
{
  return true;
}

void raspicam_release(CameraHandle)
{
}

bool raspicam_grab(CameraHandle)
{
  return true;
}

void raspicam_retrieve(CameraHandle handle, unsigned char* data)
{
  unsigned char *cam = (unsigned char *)handle;
  memcpy(data, cam, 640*480*3);
}

void raspicam_set_dims(CameraHandle, int w, int h)
{
  std::cout << "set dim " << w << " " << h << std::endl;
}
void raspicam_set_brightness(CameraHandle, int b)
{
  std::cout << "set bright " << b << std::endl;
}
void raspicam_set_saturation(CameraHandle, int ss)
{
  std::cout << "set sat " << ss << std::endl;
}

void raspicam_set_contrast(CameraHandle, int c) {
  std::cout << "set contrast " << c << std::endl;
}

// values 0 to 8
void raspicam_set_whitebalance(CameraHandle, float fr, float fb) {
  std::cout << "set wb " << fr << fb << std::endl;
}

// value 100 to 800
void raspicam_set_ISO(CameraHandle, int iso) {
  std::cout << "set iso " << iso << std::endl;
}

// value 0 to 33000
void raspicam_set_shutterspeed(CameraHandle, int nanosecs)
{
  std::cout << "set shutter" << nanosecs << std::endl;
}

