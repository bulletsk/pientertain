#ifndef PIENTERTAIN_HH
#define PIENTERTAIN_HH

#include "huestream.hh"
#include "hueauthentication.hh"
#include "videosource.hh"
#include "restserver.hh"

class PiEntertain : public QObject {
  Q_OBJECT

public:
  PiEntertain();
  virtual ~PiEntertain();

  void start();
  void startServer();

public slots:
  void startStreaming();
  void stopStreaming();
  void shutDown();
  void onStreamingActive(bool on);
  void onStreamEstablished(bool on);
  void onNewColors( const QVector<QColor> &colorVector );

protected slots:
  void sendCurrentPacket();

private:
  QVector3D mixColorForPosition( const QVector3D &pos, const QVector<QColor> &colorVector );
  QVector3D colorToVector(const QColor &c) const;

  HueAuthentication m_auth;
  HueStream m_stream;
  QTimer m_timer;
  int m_frameNumber;
  VideoSource *m_videoSource;

  RESTServer m_server;

  LightPacket m_currentPacket;

  QVector< QList< QVector3D > > m_prevColors;

};



#endif // PIENTERTAIN_HH
