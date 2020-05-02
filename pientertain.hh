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

  void start();

  void startServer();

  virtual ~PiEntertain();

public slots:
  void startStreaming();

  void stopStreaming();

  void shutDown();

  void onStreamingActive(bool on);

  void onStreamEstablished(bool on);
  void onTimer();

  void onNewColors( QVector<QColor> colorVector );

  QVector3D mixColorForPosition( const QVector3D &pos, const QVector<QColor> &colorVector );
 private:

  QVector3D colorToVector(const QColor &c) const;
  QColor vectorToColor(const QVector3D &c) const;

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
