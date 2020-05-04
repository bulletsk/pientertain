#include "videosourceraspberrycam.hh"
#if WIN32
#include <raspicaminterfacedummy.h>
#else
#include <raspicam/raspicaminterface.h>
#endif
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include <QCoreApplication>

static const int s_defaultBrightness = 50;
static const int s_defaultSaturation = 100;
static const int s_defaultContrast = 0;
static const int s_defaultWb_r = 200;
static const int s_defaultWb_b = 200;
static const int s_defaultIso = 400;
static const int s_defaultShutter = 40000;
static const int s_defaultImageWidth = 640;
static const int s_defaultImageHeight = 480;
static const int s_maxShutter = 330000;
static const int s_minIso = 100;
static const int s_maxIso = 800;
static const int s_maxWb = 800;
static const int s_minRangeValue = -100;
static const int s_maxRangeValue = 100;

VideoSourceRaspberryCam::VideoSourceRaspberryCam(const QString &sourceIdentifier, QObject *parent) : VideoSource(sourceIdentifier, parent), m_camera(nullptr)
{
  readSettings();
}

VideoSourceRaspberryCam::~VideoSourceRaspberryCam() {
  writeSettings();
  shutdown();
}

void VideoSourceRaspberryCam::readSettings()
{
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
  settings.beginGroup("camera");
  m_settings["brightness"] = settings.value("brightness", s_defaultBrightness).toInt();
  m_settings["saturation"] = settings.value("saturation", s_defaultSaturation).toInt();
  m_settings["contrast"] = settings.value("contrast", s_defaultContrast).toInt();
  m_settings["whitebalance_r"] = settings.value("whitebalance_r", s_defaultWb_r).toInt();
  m_settings["whitebalance_b"] = settings.value("whitebalance_b", s_defaultWb_b).toInt();
  m_settings["iso"] = settings.value("iso", s_defaultIso).toInt();
  m_settings["shutter"] = settings.value("shutter", s_defaultShutter).toInt();
  m_settings["width"] = settings.value("width", s_defaultImageWidth).toInt();
  m_settings["height"] = settings.value("height", s_defaultImageHeight).toInt();
  settings.endGroup();
}

void VideoSourceRaspberryCam::writeSettings()
{
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
  settings.beginGroup("camera");
  settings.setValue("brightness", m_settings["brightness"].toInt());
  settings.setValue("saturation", m_settings["saturation"].toInt());
  settings.setValue("contrast", m_settings["contrast"].toInt());
  settings.setValue("whitebalance_r", m_settings["whitebalance_r"].toInt());
  settings.setValue("whitebalance_b", m_settings["whitebalance_b"].toInt());
  settings.setValue("iso", m_settings["iso"].toInt());
  settings.setValue("shutter", m_settings["shutter"].toInt());
  settings.setValue("width", m_settings["width"].toInt());
  settings.setValue("height", m_settings["height"].toInt());
  settings.endGroup();
}

void VideoSourceRaspberryCam::setCameraSettings ( const QJsonObject &json ) {

  VideoSource::setCameraSettings(json);
  if (json.contains("brightness")) {
    int value = qBound(0, json.value("brightness").toInt(), s_maxRangeValue);
    m_settings["brightness"] = value;
    if (m_camera) {
      raspicam_set_brightness(m_camera, value);
    }
  }

  if (json.contains("saturation")) {
    int value = qBound(s_minRangeValue, json.value("saturation").toInt(), s_maxRangeValue);
    m_settings["saturation"] = value;
    if (m_camera) {
      raspicam_set_saturation(m_camera, value);
    }
  }

  if (json.contains("contrast")) {
    int value = qBound(s_minRangeValue, json.value("contrast").toInt(), s_maxRangeValue);
    m_settings["contrast"] = value;
    if (m_camera) {
      raspicam_set_contrast(m_camera, value);
    }
  }

  if (json.contains("whitebalance_r") && json.contains("whitebalance_b")) {
    int r = qBound(0, json.value("whitebalance_r").toInt(), s_maxWb);
    int b = qBound(0, json.value("whitebalance_b").toInt(), s_maxWb);
    m_settings["whitebalance_r"] = r;
    m_settings["whitebalance_b"] = b;
    if (m_camera) {
      float fr = static_cast<float>(r)/100.0f;
      float fb = static_cast<float>(b)/100.0f;
      raspicam_set_whitebalance(m_camera, fr, fb);
    }
  }

  if (json.contains("iso")) {
    int value = qBound(s_minIso, json.value("iso").toInt(), s_maxIso);
    m_settings["iso"] = value;
    if (m_camera) {
      raspicam_set_ISO(m_camera, value);
    }
  }


  if (json.contains("shutter")) {
    int value = qBound(0, json.value("shutter").toInt(), s_maxShutter);
    m_settings["shutter"] = value;
    if (m_camera) {
      raspicam_set_shutterspeed(m_camera, value);
    }
  }
  emit cameraSettingsChanged(m_settings);
}


void VideoSourceRaspberryCam::nextImage() {

  bool ok = raspicam_grab(m_camera);
  if (!ok) {
    emit statusChanged("grab image error", true);
    m_requestExit = true;
    return;
  }

  raspicam_retrieve(m_camera, m_currentImage.bits() );
}

bool VideoSourceRaspberryCam::initialize()
{
  m_camera = raspicam_create_camera();

  const int width = m_settings["width"].toInt(s_defaultImageWidth);
  const int height = m_settings["height"].toInt(s_defaultImageHeight);

  raspicam_set_dims(m_camera, width, height);

  setCameraSettings(m_settings);

  bool ret = raspicam_open(m_camera);

  if (!ret) {
    emit statusChanged("camera open error", true);
    return false;
  }
  m_currentImage = QImage( width, height, QImage::Format_RGB888 );

  qDebug() << "camera is okay";
  return true;
}

bool VideoSourceRaspberryCam::shutdown()
{
  if (m_camera == nullptr) return true;
  raspicam_release(m_camera);
  raspicam_destroy_camera(m_camera);
  m_camera = nullptr;
  return true;
}
