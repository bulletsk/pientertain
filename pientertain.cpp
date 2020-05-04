#include "pientertain.hh"

const static int s_sendPacketMinimumHz = 20;

PiEntertain::PiEntertain()
{
  QString imgName = QCoreApplication::applicationDirPath() + "/colortestimage.png";
  m_videoSource = VideoSource::createVideoSource( imgName, VideoSource::Camera );
  connect(m_videoSource, &VideoSource::newColors, this, &PiEntertain::onNewColors, Qt::QueuedConnection);
  connect(m_videoSource, &VideoSource::statusChanged, &m_server, &RESTServer::onVideoStatus);
  connect(m_videoSource, &VideoSource::latestImage, &m_server, &RESTServer::onVideoImage);

  connect(&m_auth, &HueAuthentication::streamingActive, this, &PiEntertain::onStreamingActive);
  connect(&m_auth, &HueAuthentication::statusChanged, &m_server, &RESTServer::onBridgeStatus);
  connect(&m_stream, &HueStream::streamEstablished, this, &PiEntertain::onStreamEstablished);
  connect(&m_stream, &HueStream::statusChanged, &m_server, &RESTServer::onStreamStatus);

  connect(&m_server, &RESTServer::requestStart, this, &PiEntertain::startStreaming);
  connect(&m_server, &RESTServer::requestStop, this, &PiEntertain::stopStreaming);
  connect(&m_server, &RESTServer::requestShutdown, this, &PiEntertain::shutDown);
  connect(&m_server, &RESTServer::requestImage, m_videoSource, &VideoSource::onRequestImage);
  connect(&m_server, &RESTServer::cornersChanged, m_videoSource, &VideoSource::setCorners);
  connect(&m_server, &RESTServer::cameraSettingsChanged, m_videoSource, &VideoSource::setCameraSettings);
  connect(m_videoSource, &VideoSource::cameraSettingsChanged, &m_server, &RESTServer::onCameraSettingsChanged);
  connect(m_videoSource, &VideoSource::cornersChanged, &m_server, &RESTServer::onCornersChanged);

  connect(&m_timer, &QTimer::timeout, this, &PiEntertain::sendCurrentPacket);
}

void PiEntertain::start() {
  startStreaming();
}

void PiEntertain::startServer() {
  m_server.startServer();
}

PiEntertain::~PiEntertain()
{
  delete m_videoSource;
}

void PiEntertain::startStreaming() {
  m_auth.startAuthentication();
}

void PiEntertain::stopStreaming() {
  m_timer.stop();
  m_stream.stop();
  m_auth.stopStreaming();
}

void PiEntertain::shutDown() {
  stopStreaming();
  QString shutdownCommand = "sudo /sbin/shutdown";
#ifdef WIN32
  qDebug() << shutdownCommand;
#else
  QProcess::execute(shutdownCommand);
#endif
  QCoreApplication::exit(0);
}

void PiEntertain::onStreamingActive(bool on)
{
  if (on) {
    qDebug() << "streaming started";

    m_stream.connectStream(m_auth.userName(), m_auth.clientKey(), m_auth.hueBridgeIp());
  } else {
    qDebug() << "streaming finished, framenr" << m_frameNumber;
    m_timer.stop();
  }
}

void PiEntertain::onStreamEstablished(bool on)
{
  if (on) {
    m_timer.start(1000 / s_sendPacketMinimumHz);
    m_videoSource->start();
  } else {
    m_videoSource->stop();
  }
}

void PiEntertain::sendCurrentPacket() {
  const LightGroup group = m_auth.lightGroup(0);
  const QVector<Light> lights = group.lights();

  if (m_currentPacket.numLights()==0) {
    // send default packet
    for (const Light &light : lights) {
      m_currentPacket.addLightData(light.id, 0,0,0);
    }
  }
  m_currentPacket.setSequenceNumber(m_frameNumber);
  m_stream.send(m_currentPacket);
  m_frameNumber++;
  m_timer.start(1000 / s_sendPacketMinimumHz);
}

void PiEntertain::onNewColors( const QVector<QColor> &colorVector )
{
  LightGroup group = m_auth.lightGroup(0);
  QVector<Light> lights = group.lights();

  if (m_prevColors.size() < lights.size()) {
    m_prevColors.resize(lights.size());
  }

  m_currentPacket = LightPacket();

  for (int i=0;i<lights.size();i++) {
    Light light = lights[i];
    QVector3D mixedColor = mixColorForPosition( light.pos, colorVector );

    int smooth = m_videoSource->smooth();
    if (smooth > 0) {
      m_prevColors[i].push_back(mixedColor);
    }
    while (m_prevColors[i].size() > smooth) {
      m_prevColors[i].pop_front();
    }
    if (smooth > 0 && m_prevColors[i].size()>0) {
      mixedColor = std::accumulate(m_prevColors[i].begin(), m_prevColors[i].end(), QVector3D(0.0f,0.0f,0.0f));
      mixedColor /= static_cast<float>(m_prevColors[i].size());
    }
    m_currentPacket.addLightData(light.id,
                                 static_cast<uint16_t>(mixedColor.x()*65535.0f),
                                 static_cast<uint16_t>(mixedColor.y()*65535.0f),
                                 static_cast<uint16_t>(mixedColor.z()*65535.0f));
  }
}


QVector3D PiEntertain::mixColorForPosition( const QVector3D &pos, const QVector<QColor> &colorVector )
{
  QColor lefttop, righttop,
      leftbottom, rightbottom;

  QVector3D ipos = pos;

  if (pos.x() < 0) {
    // interpolate left to center
    if (pos.z() < 0) {
      // interpolate bottom to center
      lefttop = colorVector[ VideoSource::CenterLeft ];
      righttop = colorVector[ VideoSource::CenterCenter ];
      leftbottom = colorVector[ VideoSource::BottomLeft ];
      rightbottom = colorVector[ VideoSource::BottomCenter ];
      ipos[2] += 1.0f;
    } else {
      // interpolate top to center
      lefttop = colorVector[ VideoSource::TopLeft ];
      righttop = colorVector[ VideoSource::TopCenter ];
      leftbottom = colorVector[ VideoSource::CenterLeft ];
      rightbottom = colorVector[ VideoSource::CenterCenter ];
    }
    ipos[0] = qMax(0.0f, (2.0f*ipos[0]) + 1.0f);
  } else {
    // interpolate center to right
    if (pos.z() < 0) {
      // interpolate bottom to center
      lefttop = colorVector[ VideoSource::CenterCenter ];
      righttop = colorVector[ VideoSource::CenterRight ];
      leftbottom = colorVector[ VideoSource::BottomCenter ];
      rightbottom = colorVector[ VideoSource::BottomRight ];
      ipos += QVector3D( 0.0f,0.0f,1.0f );
    } else {
      // interpolate top to center
      lefttop = colorVector[ VideoSource::TopCenter ];
      righttop = colorVector[ VideoSource::TopRight ];
      leftbottom = colorVector[ VideoSource::CenterCenter ];
      rightbottom = colorVector[ VideoSource::CenterRight ];
    }
    ipos[0] = qMin(1.0f, 2.0f*ipos[0]);
  }

  ipos.setZ( 1.0f-ipos.z() );

  // interpolate colors...

  QVector3D c1 = colorToVector(lefttop);
  QVector3D c2 = colorToVector(righttop);
  QVector3D c3 = colorToVector(leftbottom);
  QVector3D c4 = colorToVector(rightbottom);

  QVector3D cX1 = (1.0f-ipos.x()) * c1 + ipos.x() * c2;
  QVector3D cX2 = (1.0f-ipos.x()) * c3 + ipos.x() * c4;

  QVector3D cFinal = (1.0f-ipos.z()) * cX1 + ipos.z() * cX2;

  return cFinal;

}

QVector3D PiEntertain::colorToVector(const QColor &c) const
{
  return QVector3D( c.redF(), c.greenF(), c.blueF() );
}
