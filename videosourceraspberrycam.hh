#ifndef VIDEOSOURCERASPBERRYCAM_HH
#define VIDEOSOURCERASPBERRYCAM_HH

#include <videosource.hh>

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

  void *m_camera;

};

#endif // VIDEOSOURCERASPBERRYCAM_HH
