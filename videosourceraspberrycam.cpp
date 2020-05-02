#include "videosourceraspberrycam.hh"
#if WIN32
#include <raspicaminterfacedummy.h>
#else
#include <raspicam/raspicaminterface.h>
#endif
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

VideoSourceRaspberryCam::VideoSourceRaspberryCam(QString sourceIdentifier, QObject *parent) : VideoSource(sourceIdentifier, parent), m_camera(nullptr)
{
  // default settings
  m_settings["brightness"] = 50;
  m_settings["saturation"] = 100;
  m_settings["contrast"] = 0;
  m_settings["whitebalance_r"] = 2;
  m_settings["whitebalance_b"] = 2;
  m_settings["iso"] = 400;
  m_settings["shutter"] = 40000;
}

VideoSourceRaspberryCam::~VideoSourceRaspberryCam() {
  shutdownCamera();
}

void VideoSourceRaspberryCam::setCameraSettings ( QJsonObject json ) {

  m_settings = json;

  if (json.contains("brightness")) {
    int value = qBound(0, json.value("brightness").toInt(), 100);
    m_settings["brightness"] = value;
    if (m_camera) {
      raspicam_set_brightness(m_camera, value);
    }
  }

  if (json.contains("saturation")) {
    int value = qBound(-100, json.value("brightness").toInt(), 100);
    m_settings["saturation"] = value;
    if (m_camera) {
      raspicam_set_saturation(m_camera, value);
    }
  }

  if (json.contains("contrast")) {
    int value = qBound(-100, json.value("contrast").toInt(), 100);
    m_settings["contrast"] = value;
    if (m_camera) {
      raspicam_set_contrast(m_camera, value);
    }
  }

  if (json.contains("whitebalance_r") && json.contains("whitebalance_b")) {
    int r = qBound(-100, json.value("whitebalance_r").toInt(), 100);
    int b = qBound(-100, json.value("whitebalance_b").toInt(), 100);
    m_settings["whitebalance_r"] = r;
    m_settings["whitebalance_b"] = b;
    if (m_camera) {
      raspicam_set_whitebalance(m_camera, r, b);
    }
  }

  if (json.contains("iso")) {
    int value = qBound(100, json.value("iso").toInt(), 800);
    m_settings["iso"] = value;
    if (m_camera) {
      raspicam_set_ISO(m_camera, value);
    }
  }


  if (json.contains("shutter")) {
    int value = qBound(0, json.value("shutter").toInt(), 330000);
    m_settings["shutter"] = value;
    if (m_camera) {
      raspicam_set_shutterspeed(m_camera, value);
    }
  }
  emit cameraSettingsChanged(m_settings);
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

  setCameraSettings(m_settings);

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
