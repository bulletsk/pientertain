#include <iostream>
#include "hueauthentication.hh"
#include "huestream.hh"
#include "lightpacket.hh"

#include <QCoreApplication>


class StreamClient : public QObject {
  Q_OBJECT

public:
  StreamClient()
  {
    connect(&m_auth, &HueAuthentication::streamingActive, this, &StreamClient::onStreamingActive);
    connect(&m_stream, &HueStream::streamEstablished, this, &StreamClient::onStreamEstablished);
    connect(&m_stream, &HueStream::connectionError, this, &StreamClient::onConnectionError);

    m_frameNumber = 0;
  }

  ~StreamClient()
  {}

  void start() {
    m_auth.startAuthentication();
  }


public slots:

  void stopStreaming() {
    m_timer.stop();
    m_stream.stop();
    m_auth.stopStreaming();
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

  void onStreamEstablished()
  {
    qDebug() << "send something!";
    // send something :)
    //m_stream

    QTimer::singleShot(5000, this, &StreamClient::stopStreaming);

    connect(&m_timer, &QTimer::timeout, this, &StreamClient::onTimer);
    m_timer.start(10);

  }

  void onConnectionError(QString err) {
    qDebug() << "received error:" << err;
    QCoreApplication::exit(0);
  }

  void onTimer() {
    unsigned short usValue = (unsigned short)m_frameNumber*160;

    LightGroup group = m_auth.lightGroup(0);
    QVector<LightGroup::Light> lights = group.lights();

    LightPacket lp;
    for (int i=0;i<lights.size();i++) {
      LightGroup::Light light = lights[i];
      lp.addLightData(light.id, usValue,usValue,usValue);
    }
    lp.setSequenceNumber(m_frameNumber);
    m_stream.send(lp);
    m_frameNumber++;
  }

 private:
  HueAuthentication m_auth;
  HueStream m_stream;
  QTimer m_timer;
  int m_frameNumber;
};




int main(int argc, char *argv[]) {
    std::cout << "hello pi" << std::endl;

    QCoreApplication::setOrganizationName("appkellner");
    QCoreApplication::setOrganizationDomain("www.appkellner.de");
    QCoreApplication::setApplicationName("PiEntertain");

    QCoreApplication app(argc, argv);

    StreamClient client;
    client.start();

    return app.exec();
}

#include "main.moc"
