#ifndef VIDEOSOURCERASPBERRYCAM_HH
#define VIDEOSOURCERASPBERRYCAM_HH

#include <videosource.hh>
#include <raspicam/raspicaminterface_global.h>

class VideoSourceRaspberryCam : public VideoSource
{
  Q_OBJECT
public:
  VideoSourceRaspberryCam(QString sourceIdentifier="", QObject *parent = nullptr);

  virtual ~VideoSourceRaspberryCam() override;

  virtual void nextImage() override;

protected:



  bool initializeCamera();

  bool shutdownCamera();

  CameraHandle m_camera;

};

#endif // VIDEOSOURCERASPBERRYCAM_HH
