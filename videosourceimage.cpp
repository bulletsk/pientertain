#include "videosourceimage.hh"
#include <QDebug>

VideoSourceImage::VideoSourceImage(QString sourceIdentifier, QObject *parent)
  : VideoSource(sourceIdentifier, parent)
{

}

void VideoSourceImage::nextImage()
{


  if (m_currentImage.isNull()) {

    bool ok = m_currentImage.load(m_identifier);
    if (!ok) {
      qDebug() << "could not load image" << m_identifier;
      emit statusChanged("could not load image " + m_identifier, true);
    } else {
      emit statusChanged("image loaded", false);
    }

  }

  QThread::msleep(2000);


}
