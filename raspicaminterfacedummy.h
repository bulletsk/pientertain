#ifndef RASPICAMDUMMYINTERFACE_H
#define RASPICAMDUMMYINTERFACE_H

typedef void* CameraHandle;

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

// values 0 to 100
void raspicam_set_brightness(CameraHandle handle, int b);

// values -100 to 100
void raspicam_set_saturation(CameraHandle handle, int s);
void raspicam_set_contrast(CameraHandle handle, int c);

// values 0 to 8
void raspicam_set_whitebalance(CameraHandle handle, float r, float b);

// value 100 to 800
void raspicam_set_ISO(CameraHandle handle, int iso);

// value 0 to 330000
void raspicam_set_shutterspeed(CameraHandle handle, int nanosecs);


#ifdef __cplusplus
}
#endif

#endif // RASPICAMDUMMYINTERFACE_H
