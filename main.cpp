#include <iostream>
#include "hueauthentication.hh"
#include "huestream.hh"
#include "lightpacket.hh"
#include "videosource.hh"
#include "restserver.hh"

#include <QCoreApplication>


class StreamClient : public QObject {
  Q_OBJECT

public:
  StreamClient()
  {
    QString imgName = QCoreApplication::applicationDirPath() + "/colortestimage.png";
    m_videoSource = VideoSource::createVideoSource( imgName, VideoSource::Image );
    connect(m_videoSource, &VideoSource::newColors, this, &StreamClient::onNewColors, Qt::QueuedConnection);
    connect(m_videoSource, &VideoSource::statusChanged, &m_server, &RESTServer::onVideoStatus);

    connect(&m_auth, &HueAuthentication::streamingActive, this, &StreamClient::onStreamingActive);
    connect(&m_auth, &HueAuthentication::statusChanged, &m_server, &RESTServer::onBridgeStatus);
    connect(&m_stream, &HueStream::streamEstablished, this, &StreamClient::onStreamEstablished);
    connect(&m_stream, &HueStream::statusChanged, &m_server, &RESTServer::onStreamStatus);

    connect(&m_server, &RESTServer::requestStart, this, &StreamClient::startStreaming);
    connect(&m_server, &RESTServer::requestStop, this, &StreamClient::stopStreaming);
    connect(&m_server, &RESTServer::requestShutdown, this, &StreamClient::shutDown);
    connect(&m_server, &RESTServer::cornersChanged, m_videoSource, &VideoSource::setCorners);

    connect(&m_timer, &QTimer::timeout, this, &StreamClient::onTimer);

    m_frameNumber = 0;
  }

  void start() {
    startStreaming();
  }

  void startServer() {
    m_server.startServer();
  }

  ~StreamClient()
  {
    delete m_videoSource;
  }

public slots:
  void startStreaming() {
    m_auth.startAuthentication();
  }

  void stopStreaming() {
    m_timer.stop();
    m_stream.stop();
    m_auth.stopStreaming();
  }

  void shutDown() {
    QCoreApplication::exit(0);
  }

  void onStreamingActive(bool on)
  {
    if (on) {
      qDebug() << "streaming started";

      m_stream.connectStream(m_auth.userName(), m_auth.clientKey());
    } else {
      qDebug() << "streaming finished, framenr" << m_frameNumber;
      m_timer.stop();
      QCoreApplication::exit(0);
    }
  }

  void onStreamEstablished(bool on)
  {
    if (on) qDebug() << "send something!";
    // send something :)
    //m_stream

    if (on) {
      m_timer.start(10);
      m_videoSource->start();
    } else {
      m_videoSource->stop();
    }
  }

  void onTimer() {
    LightGroup group = m_auth.lightGroup(0);
    QVector<LightGroup::Light> lights = group.lights();

    if (m_currentPacket.numLights()==0) {
      // send default packet
      for (int i=0;i<lights.size();i++) {
        LightGroup::Light light = lights[i];
        m_currentPacket.addLightData(light.id, 0,0,0);
      }
    }
    m_currentPacket.setSequenceNumber(m_frameNumber);
    m_stream.send(m_currentPacket);
    m_frameNumber++;
  }

  void onNewColors( QVector<QColor> colorVector )
  {
    LightGroup group = m_auth.lightGroup(0);
    QVector<LightGroup::Light> lights = group.lights();

    m_currentPacket = LightPacket();

    for (int i=0;i<lights.size();i++) {
      LightGroup::Light light = lights[i];
      QVector3D mixedColor = mixColorForPosition( light.pos, colorVector );
//      qDebug() << fakePos << mixedColor;
      m_currentPacket.addLightData(light.id, (uint16_t)(mixedColor.x()*65535.0),(uint16_t)(mixedColor.y()*65535.0),(uint16_t)(mixedColor.z()*65535.0));
    }
  }


  QVector3D mixColorForPosition( const QVector3D &pos, const QVector<QColor> &colorVector )
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
        //ipos.z() += QVector3D( 1.0,0.0,1.0 );
        ipos[2] += 1.0;
      } else {
        // interpolate top to center
        lefttop = colorVector[ VideoSource::TopLeft ];
        righttop = colorVector[ VideoSource::TopCenter ];
        leftbottom = colorVector[ VideoSource::CenterLeft ];
        rightbottom = colorVector[ VideoSource::CenterCenter ];
      }
      ipos[0] = qMax(0.0, (2.0*ipos[0]) + 1.0);
    } else {
      // interpolate center to right
      if (pos.z() < 0) {
        // interpolate bottom to center
        lefttop = colorVector[ VideoSource::CenterCenter ];
        righttop = colorVector[ VideoSource::CenterRight ];
        leftbottom = colorVector[ VideoSource::BottomCenter ];
        rightbottom = colorVector[ VideoSource::BottomRight ];
        ipos += QVector3D( 0.0,0.0,1.0 );
      } else {
        // interpolate top to center
        lefttop = colorVector[ VideoSource::TopCenter ];
        righttop = colorVector[ VideoSource::TopRight ];
        leftbottom = colorVector[ VideoSource::CenterCenter ];
        rightbottom = colorVector[ VideoSource::CenterRight ];
      }
      ipos[0] = qMin(1.0, 2.0*ipos[0]);
    }

    ipos.setZ( 1.0-ipos.z() );

    // interpolate colors...

    QVector3D c1 = colorToVector(lefttop);
    QVector3D c2 = colorToVector(righttop);
    QVector3D c3 = colorToVector(leftbottom);
    QVector3D c4 = colorToVector(rightbottom);

    QVector3D cX1 = (1.0-ipos.x()) * c1 + ipos.x() * c2;
    QVector3D cX2 = (1.0-ipos.x()) * c3 + ipos.x() * c4;

    QVector3D cFinal = (1.0-ipos.z()) * cX1 + ipos.z() * cX2;

    return cFinal;

  }


 private:

  QVector3D colorToVector(const QColor &c) const
  {
    return QVector3D( c.redF(), c.greenF(), c.blueF() );
  }
  QColor vectorToColor(const QVector3D &c) const
  {
    QColor ret;
    ret.setRgb( (int)c.x()*255.0f, (int)c.y()*255.0f, (int)c.z()*255.0f );
    return ret;
  }

  HueAuthentication m_auth;
  HueStream m_stream;
  QTimer m_timer;
  int m_frameNumber;
  VideoSource *m_videoSource;

  RESTServer m_server;

  LightPacket m_currentPacket;
};




int main(int argc, char *argv[]) {
  std::cout << "hello pi" << std::endl;

  QCoreApplication::setOrganizationName("appkellner");
  QCoreApplication::setOrganizationDomain("www.appkellner.de");
  QCoreApplication::setApplicationName("PiEntertain");

  QCoreApplication app(argc, argv);

  qRegisterMetaType< QVector<QColor> >();
  qRegisterMetaType< QVector<QPoint> >();

  StreamClient client;
//  client.start();
  client.startServer();

  return app.exec();
}

#include "main.moc"
