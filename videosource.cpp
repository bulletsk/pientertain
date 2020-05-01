#include "videosource.hh"
#include "videosourceimage.hh"
#include "videosourceraspberrycam.hh"
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>

VideoSource *VideoSource::createVideoSource(QString identifier, VideoSourceType type) {
  switch (type) {
  case Image:
    return new VideoSourceImage(identifier, nullptr);
  case Camera:
    return new VideoSourceRaspberryCam(identifier, nullptr);
  default:
    break;
  }
  qDebug() << "not yet implemented";
  return nullptr;
}


VideoSource::VideoSource(QString sourceIdentifier, QObject *parent) : QThread(parent), m_identifier(sourceIdentifier), m_requestExit(false)
{

  readSettings();
}

VideoSource::~VideoSource() {
  writeSettings();
  if (isRunning()) {
    stop();
  }
}

void VideoSource::readSettings()
{
  m_corners.clear();
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName());
  settings.beginGroup("corners");
  for (int i=1;i<=4;i++) {
    QPoint p = settings.value("point"+QString::number(i), QPoint(-1,-1)).toPoint();
    if ( p.x() < 0) {
      m_corners.clear();
      break;
    }
    m_corners.append(p);
  }
  settings.endGroup();
}

void VideoSource::writeSettings()
{
  if (m_corners.size() != 4) {
    return;
  }
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName());
  settings.beginGroup("corners");
  for (int i=1;i<=4;i++) {
    settings.setValue("point"+QString::number(i), m_corners[i-1]);
  }
  settings.endGroup();
}

void VideoSource::stop()
{
  m_requestExit = true;
  wait();
}


QVector<QPoint> VideoSource::corners() const
{
  return m_corners;
}
void VideoSource::setCorners( const QVector<QPoint> &corners)
{
  if (corners.size()!=4) {
    return;
  }
  m_corners = corners;
}

QSize VideoSource::imageSize() const {
  return m_currentImage.size();
}

QImage VideoSource::currentImage() const {
  return m_currentImage;
}

void VideoSource::onRequestImage() {
  m_imageLock.lock();
  m_latestImage = m_currentImage;
  m_imageLock.unlock();
  emit latestImage(m_latestImage);
}

void VideoSource::run() {

  m_colors.resize((int)CornerLast);

  while (!m_requestExit) {
    m_imageLock.lock();
    nextImage();
    m_imageLock.unlock();
    calculateColors();
    emit newColors(m_colors);

  }

  m_requestExit = false;

}

void VideoSource::calculateColors()
{

  if (m_currentImage.isNull()) {
    return;
  }

  QVector<QPoint> curCorners;

  if (m_corners.size() != 4) {
    curCorners.append( QPoint(0,0) );
    curCorners.append( QPoint( m_currentImage.width()-1, 0) );
    curCorners.append( QPoint( 0, m_currentImage.height()-1) );
    curCorners.append( QPoint( m_currentImage.width()-1, m_currentImage.height()-1) );
  } else {
    curCorners = m_corners;
  }

  if (m_currentImage.format() != QImage::Format_RGB888) {
    qDebug() << "format not rgb888";
    return;
  }

  QList<QPoint> points;
  points << curCorners[CCTopLeft]
         << curCorners[CCTopLeft] + ((curCorners[CCTopRight] - curCorners[CCTopLeft])/2)
         << curCorners[CCTopRight]
         << curCorners[CCTopLeft] + ((curCorners[CCBottomLeft] - curCorners[CCTopLeft])/2)
         << curCorners[CCTopLeft] + ((curCorners[CCBottomRight] - curCorners[CCTopLeft])/2)
         << curCorners[CCTopRight] + ((curCorners[CCBottomRight] - curCorners[CCTopRight])/2)
         << curCorners[CCBottomLeft]
         << curCorners[CCBottomLeft] + ((curCorners[CCBottomRight] - curCorners[CCBottomLeft])/2)
         << curCorners[CCBottomRight];

  int corner = 0;
  for (QPoint point : points) {

    //qDebug() << "CORNER" << point;

    int xl = qMax(0, point.x()-50);
    int yl = qMax(0, point.y()-50);

    int xr = qMin(m_currentImage.width(), point.x()+50);
    int yr = qMin(m_currentImage.height(), point.y()+50);

    int num = 0;
    int r = 0;
    int g = 0;
    int b = 0;

    for (int y=yl;y<yr;y++) {
      for (int x=xl;x<xr;x++) {
        QColor color = m_currentImage.pixelColor(x,y);
        r += color.red();
        g += color.green();
        b += color.blue();
        ++num;
      }
    }

    r /= (float)num;
    g /= (float)num;
    b /= (float)num;

    m_colors[corner] = QColor(r,g,b);

    ++corner;
  }
}
