#ifndef VIDEOSOURCEIMAGE_HH
#define VIDEOSOURCEIMAGE_HH

#include <videosource.hh>

class VideoSourceImage : public VideoSource
{
  Q_OBJECT
public:
  VideoSourceImage(QString sourceIdentifier="", QObject *parent=nullptr);

protected:
  virtual void nextImage();

};

#endif // VIDEOSOURCEIMAGE_HH
