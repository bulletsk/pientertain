#ifndef RASPICAMINTERFACE_H
#define RASPICAMINTERFACE_H

#include "raspicaminterface_global.h"
#include <raspicam/raspicamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

CameraHandle raspicam_create_camera();
bool raspicam_destroy_camera(CameraHandle handle);

bool raspicam_open(CameraHandle handle);
void raspicam_release(CameraHandle handle);

bool raspicam_grab(CameraHandle handle);
void raspicam_retrieve(CameraHandle handle, unsigned char* data);

void raspicam_set_dims(CameraHandle handle, int width, int height);
void raspicam_set_brightness(CameraHandle handle, int b);
void raspicam_set_saturation(CameraHandle handle, int s);

#ifdef __cplusplus
}
#endif

#endif // RASPICAMINTERFACE_H
