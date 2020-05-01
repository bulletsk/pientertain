#ifndef VIDEOSOURCERASPBERRYCAM_HH
#define VIDEOSOURCERASPBERRYCAM_HH

#include <videosource.hh>
#if WIN32
#include <raspicaminterfacedummy.h>
#else
#include <raspicam/raspicaminterface_global.h>
#endif

class VideoSourceRaspberryCam : public VideoSource
{
  Q_OBJECT
public:
  VideoSourceRaspberryCam(QString sourceIdentifier="", QObject *parent = nullptr);

  virtual ~VideoSourceRaspberryCam() override;

  virtual void nextImage() override;

public slots:
  void setCameraSettings ( QJsonObject json ) override;

protected:



  bool initializeCamera();

  bool shutdownCamera();

  CameraHandle m_camera;

  QJsonObject m_settings;

};

#endif // VIDEOSOURCERASPBERRYCAM_HH
