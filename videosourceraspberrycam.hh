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
  VideoSourceRaspberryCam(const QString &sourceIdentifier="", QObject *parent = nullptr);

  virtual ~VideoSourceRaspberryCam() override;

  virtual void nextImage() override;

  void readSettings();
  void writeSettings();

public slots:
  void setCameraSettings (const QJsonObject &json ) override;

protected:



  bool initializeCamera();

  bool shutdownCamera();

  CameraHandle m_camera;


};

#endif // VIDEOSOURCERASPBERRYCAM_HH
