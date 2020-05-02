#include "videosourceimage.hh"
#include <QDebug>

VideoSourceImage::VideoSourceImage(const QString &sourceIdentifier, QObject *parent)
  : VideoSource(sourceIdentifier, parent)
{

}

VideoSourceImage::~VideoSourceImage() {
}

void VideoSourceImage::nextImage()
{


  if (m_currentImage.isNull()) {

    QImage tmp;

    bool ok = tmp.load(m_identifier);
    if (!ok) {
      qDebug() << "could not load image" << m_identifier;
      emit statusChanged("could not load image " + m_identifier, true);
    } else {
      m_currentImage = tmp.convertToFormat(QImage::Format_RGB888);

      emit statusChanged("image loaded", false);
    }

  }

  QThread::msleep(2000);


}
