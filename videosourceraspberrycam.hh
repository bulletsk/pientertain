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

  virtual bool initialize() override;
  virtual bool shutdown() override;

  void readSettings();
  void writeSettings();

public slots:
  void setCameraSettings (const QJsonObject &json ) override;

protected:
  CameraHandle m_camera;


};

#endif // VIDEOSOURCERASPBERRYCAM_HH
