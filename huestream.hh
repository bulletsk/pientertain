#ifndef HUESTREAM_HH
#define HUESTREAM_HH

#include <QDtls>
#include "lightpacket.hh"

class HueStream : public QObject
{
  Q_OBJECT
public:
  explicit HueStream(QObject *parent = nullptr);

  void connectStream(const QString &username, const QString &clientkey, const QString &huebridge);

  virtual ~HueStream();

  void send(const LightPacket &lp);
  void stop();

signals:
  void streamEstablished(bool on);
  void timeoutError();
  void statusChanged(QString status, bool err);

protected slots:
  void onPskRequired(QSslPreSharedKeyAuthenticator *authenticator);
  void onHandshakeTimeout();
  void onDataAvailable();

private:

  QDtls *m_dtls;
  QUdpSocket *m_clientSocket;

  QString m_username;
  QString m_clientkey;

};

#endif // HUESTREAM_HH
