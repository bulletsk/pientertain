#include "videosourceimage.hh"
#include <QDebug>

static const int s_nextImageSleep = 40;

VideoSourceImage::VideoSourceImage(const QString &sourceIdentifier, QObject *parent)
  : VideoSource(sourceIdentifier, parent)
{

}

void VideoSourceImage::nextImage()
{
  QThread::msleep(s_nextImageSleep);
}

bool VideoSourceImage::initialize() {
  QImage tmp;
  bool ok = tmp.load(m_identifier);
  if (!ok) {
    qDebug() << "could not load image" << m_identifier;
    return false;
  }
  m_currentImage = tmp.convertToFormat(QImage::Format_RGB888);
  emit statusChanged("image loaded", false);
  return true;
}

bool VideoSourceImage::shutdown() {
  return true;
}

