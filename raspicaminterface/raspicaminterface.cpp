#include "raspicaminterface.h"

#include <raspicam/raspicam.h>


CameraHandle raspicam_create_camera()
{
  raspicam::RaspiCam* cam = new raspicam::RaspiCam;
  cam->setSharpness(0);
  cam->setContrast(0);
  cam->setSaturation(0);
  cam->setISO(400);
  cam->setVideoStabilization(false);
  cam->setExposureCompensation(raspicam::RASPICAM_EXPOSURE_OFF);
  cam->setAWB(raspicam::RASPICAM_AWB_OFF);
//  cam->setFrameRate(60);
  cam->setFormat( raspicam::RASPICAM_FORMAT_RGB );
  cam->setShutterSpeed(40000);
  return (CameraHandle)cam;
}
bool raspicam_destroy_camera(CameraHandle handle)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  delete cam;
  return true;
}

bool raspicam_open(CameraHandle handle)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  return cam->open();

}

void raspicam_release(CameraHandle handle)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  cam->release();
}

bool raspicam_grab(CameraHandle handle)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  return cam->grab();
}

void raspicam_retrieve(CameraHandle handle, unsigned char* data)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  cam->retrieve(data);
}

void raspicam_set_dims(CameraHandle handle, int width, int height)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  cam->setWidth(width);
  cam->setHeight(height);
}
void raspicam_set_brightness(CameraHandle handle, int b)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;

  cam->setBrightness(b);
}
void raspicam_set_saturation(CameraHandle handle, int s)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  cam->setSaturation(s);
}

void raspicam_set_contrast(CameraHandle handle, int c) {
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  cam->setContrast(c);
}

// values 0 to 8
void raspicam_set_whitebalance(CameraHandle handle, float r, float b) {
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  cam->setAWB_RB(r,b);
}

// value 100 to 800
void raspicam_set_ISO(CameraHandle handle, int iso) {
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  cam->setISO(iso);
}

// value 0 to 33000
void raspicam_set_shutterspeed(CameraHandle handle, int nanosecs)
{
  raspicam::RaspiCam *cam = (raspicam::RaspiCam *)handle;
  cam->setShutterSpeed(nanosecs);
}

