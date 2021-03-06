#ifndef VIDEOSOURCEIMAGE_HH
#define VIDEOSOURCEIMAGE_HH

#include <videosource.hh>

class VideoSourceImage : public VideoSource
{
  Q_OBJECT
public:
  VideoSourceImage(const QString &sourceIdentifier="", QObject *parent=nullptr);
  virtual ~VideoSourceImage() override = default;
protected:
  virtual void nextImage() override;
  virtual bool initialize() override;
  virtual bool shutdown() override;

};

#endif // VIDEOSOURCEIMAGE_HH
