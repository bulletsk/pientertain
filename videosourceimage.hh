#ifndef VIDEOSOURCEIMAGE_HH
#define VIDEOSOURCEIMAGE_HH

#include <videosource.hh>

class VideoSourceImage : public VideoSource
{
  Q_OBJECT
public:
  VideoSourceImage(QString sourceIdentifier="", QObject *parent=nullptr);
  virtual ~VideoSourceImage() override;
protected:
  virtual void nextImage() override;

};

#endif // VIDEOSOURCEIMAGE_HH
