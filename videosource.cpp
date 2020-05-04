#include "videosource.hh"
#include "videosourceimage.hh"
#include "videosourceraspberrycam.hh"
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QElapsedTimer>

static const int s_minArea = 5;
static const int s_maxArea = 200;
static const int s_minSmooth = 0;
static const int s_maxSmooth = 100;
static const int s_defaultAreaSize = 20;
static const int s_defaultSmooth = 0;

VideoSource *VideoSource::createVideoSource(const QString &identifier, VideoSourceType type) {
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


VideoSource::VideoSource(const QString &sourceIdentifier, QObject *parent) : QThread(parent), m_identifier(sourceIdentifier), m_requestExit(false), m_areaSize(s_defaultAreaSize), m_smoothCount(s_defaultSmooth)
{
  readSettings();
}

int VideoSource::area() const
{
  return m_areaSize;
}
int VideoSource::smooth() const
{
  return m_smoothCount;
}


VideoSource::~VideoSource() {
  writeSettings();
  if (isRunning()) {
    stop();
  }
}

void VideoSource::readSettings()
{
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
  settings.beginGroup("processing");
  m_areaSize = settings.value("area", s_defaultAreaSize).toInt();
  m_smoothCount = settings.value("smooth", s_defaultSmooth).toInt();
  m_settings["area"] = m_areaSize;
  m_settings["smooth"] = m_smoothCount;
  settings.endGroup();

  QVector<QPoint> corners;
  settings.beginGroup("corners");
  for (int i=1;i<=4;i++) {
    QPoint p = settings.value("point"+QString::number(i), QPoint(-1,-1)).toPoint();
    if ( p.x() < 0) {
      corners.clear();
      break;
    }
    corners.append(p);
  }
  settings.endGroup();
  if (corners.size() == 4) {
    setCorners(corners);
  }
}

void VideoSource::writeSettings()
{
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
  settings.beginGroup("processing");
  settings.setValue("area", m_areaSize);
  settings.setValue("smooth", m_smoothCount);
  settings.endGroup();

  if (m_corners.size() != 4) {
    return;
  }
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

  m_measurePoints.clear();

  m_measurePoints << m_corners[CCTopLeft]
         << m_corners[CCTopLeft] + ((m_corners[CCTopRight] - m_corners[CCTopLeft])/2)
         << m_corners[CCTopRight]
         << m_corners[CCTopLeft] + ((m_corners[CCBottomLeft] - m_corners[CCTopLeft])/2)
         << m_corners[CCTopLeft] + ((m_corners[CCBottomRight] - m_corners[CCTopLeft])/2)
         << m_corners[CCTopRight] + ((m_corners[CCBottomRight] - m_corners[CCTopRight])/2)
         << m_corners[CCBottomLeft]
         << m_corners[CCBottomLeft] + ((m_corners[CCBottomRight] - m_corners[CCBottomLeft])/2)
         << m_corners[CCBottomRight];
}

void VideoSource::setCameraSettings (const QJsonObject &json )
{
  if (json.contains("area")) {
    int value = qBound(s_minArea, json.value("area").toInt(), s_maxArea);
    m_settings["area"] = value;
    m_areaSize = value;
  }
  if (json.contains("smooth")) {
    int value = qBound(s_minSmooth, json.value("smooth").toInt(), s_maxSmooth);
    m_settings["smooth"] = value;
    m_smoothCount = value;
  }
}

void VideoSource::onRequestImage() {
  m_imageLock.lock();
  m_latestImage = m_currentImage;
  m_imageLock.unlock();
  emit latestImage(m_latestImage);
}

void VideoSource::run() {

  emit cornersChanged(m_corners);
  emit cameraSettingsChanged(m_settings);

  m_colors.resize(static_cast<int>(CornerLast));

  QElapsedTimer frameRateTimer;
  int frameRateCounter = 0;
  frameRateTimer.start();
  while (!m_requestExit) {
    m_imageLock.lock();
    nextImage();
    m_imageLock.unlock();
    calculateColors();
    emit newColors(m_colors);

    frameRateCounter++;
    if (frameRateCounter==100) {
      quint64 msecs = frameRateTimer.restart();
      QString fpsMessage = QString::number( (1000.0f / (msecs/100.0f)), 'g', 4) + " fps";
      frameRateCounter = 0;
      emit statusChanged( fpsMessage, false );
    }
  }

  m_requestExit = false;

}

void VideoSource::calculateColors()
{

  if (m_currentImage.isNull()) {
    return;
  }

  if (m_corners.size() != 4) {
    QVector<QPoint> defaultCorners;
    defaultCorners.append( QPoint(0,0) );
    defaultCorners.append( QPoint( m_currentImage.width()-1, 0) );
    defaultCorners.append( QPoint( 0, m_currentImage.height()-1) );
    defaultCorners.append( QPoint( m_currentImage.width()-1, m_currentImage.height()-1) );
    setCorners(defaultCorners);
  }

  if (m_currentImage.format() != QImage::Format_RGB888) {
    qDebug() << "format not rgb888";
    return;
  }

  int corner = 0;
  const int areaSize = area();

  const int width = m_currentImage.width();
  const int height = m_currentImage.height();

  const unsigned char *data = m_currentImage.constBits();

  for (QPoint point : qAsConst(m_measurePoints)) {

    const int xl = qMax(0, point.x()-areaSize);
    const int yl = qMax(0, point.y()-areaSize);

    const int xr = qMin(width, point.x()+areaSize);
    const int yr = qMin(height, point.y()+areaSize);

    int num = 0;
    int r = 0;
    int g = 0;
    int b = 0;

    int idx;
    for (int y=yl;y<yr;y++) {
      for (int x=xl;x<xr;x++) {
        idx = (y*width+x)*3;
        r += static_cast<int>(data[idx+0]);
        g += static_cast<int>(data[idx+1]);
        b += static_cast<int>(data[idx+2]);
        ++num;
      }
    }

    if (num > 0) {
      r /= num;
      g /= num;
      b /= num;
    }

    m_colors[corner] = QColor(r,g,b);

    ++corner;
  }
}
