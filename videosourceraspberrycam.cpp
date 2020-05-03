#include "videosourceraspberrycam.hh"
#if WIN32
#include <raspicaminterfacedummy.h>
#else
#include <raspicam/raspicaminterface.h>
#endif
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

static const int s_defaultBrightness = 50;
static const int s_defaultSaturation = 100;
static const int s_defaultContrast = 0;
static const int s_defaultWb_r = 200;
static const int s_defaultWb_b = 200;
static const int s_defaultIso = 400;
static const int s_defaultShutter = 40000;
static const int s_imageWidth = 640;
static const int s_imageHeight = 480;
static const int s_maxShutter = 330000;
static const int s_minIso = 100;
static const int s_maxIso = 800;
static const int s_maxWb = 800;
static const int s_minRangeValue = -100;
static const int s_maxRangeValue = 100;

VideoSourceRaspberryCam::VideoSourceRaspberryCam(const QString &sourceIdentifier, QObject *parent) : VideoSource(sourceIdentifier, parent), m_camera(nullptr)
{
  // default settings
  m_settings["brightness"] = s_defaultBrightness;
  m_settings["saturation"] = s_defaultSaturation;
  m_settings["contrast"] = s_defaultContrast;
  m_settings["whitebalance_r"] = s_defaultWb_r;
  m_settings["whitebalance_b"] = s_defaultWb_b;
  m_settings["iso"] = s_defaultIso;
  m_settings["shutter"] = s_defaultShutter;
}

VideoSourceRaspberryCam::~VideoSourceRaspberryCam() {
  shutdownCamera();
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

  bool ok;
  if (m_camera == nullptr) {
    ok = initializeCamera();
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

  raspicam_set_dims(m_camera, s_imageWidth, s_imageHeight);

  setCameraSettings(m_settings);

  bool ret = raspicam_open(m_camera);

  if (!ret) {
    emit statusChanged("camera open error", true);
    return false;
  }
  m_currentImage = QImage( s_imageWidth, s_imageHeight, QImage::Format_RGB888 );

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
