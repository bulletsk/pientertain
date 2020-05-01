#include "videosourceraspberrycam.hh"
#include <raspicam/raspicaminterface.h>
#include <QDebug>

VideoSourceRaspberryCam::VideoSourceRaspberryCam(QString sourceIdentifier, QObject *parent) : VideoSource(sourceIdentifier, parent), m_camera(nullptr)
{
}

VideoSourceRaspberryCam::~VideoSourceRaspberryCam() {
  shutdownCamera();
}

void VideoSourceRaspberryCam::nextImage() {

  bool ok = true;
  if (m_camera == nullptr) {
    bool ok = initializeCamera();
    if (!ok) {
      m_requestExit = true;
      return;
    }
  }

  ok = raspicam_grab(m_camera);
  if (!ok) {
    emit statusChanged("grab image error", true);
    m_requestExit = true;
    return;
  }

  raspicam_retrieve(m_camera, m_currentImage.bits() );
}

bool VideoSourceRaspberryCam::initializeCamera()
{
  m_camera = raspicam_create_camera();

  raspicam_set_dims(m_camera, 640, 480);
  raspicam_set_brightness(m_camera, 50);
  raspicam_set_saturation(m_camera, 0);

  bool ret = raspicam_open(m_camera);

  if (!ret) {
    emit statusChanged("camera open error", true);
    return false;
  }
  m_currentImage = QImage( 640, 480, QImage::Format_RGB888 );

  qDebug() << "camera is okay";
  return true;
}

bool VideoSourceRaspberryCam::shutdownCamera()
{
  if (m_camera == nullptr) return true;
  raspicam_release(m_camera);
  raspicam_destroy_camera(m_camera);
  m_camera = nullptr;
  return true;
}
