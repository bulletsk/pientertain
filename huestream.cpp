#include "huestream.hh"
#include <QHostAddress>
#include <QSslConfiguration>
#include <QUdpSocket>
#include <QSslCipher>
#include <QSslPreSharedKeyAuthenticator>

HueStream::~HueStream() {
  blockSignals(true);
  stop();
  blockSignals(false);
}

void HueStream::stop() {
  emit streamEstablished(false);
  emit statusChanged("stopped", false);
  if (m_dtls) {
    m_dtls->shutdown(m_clientSocket);
  }
  delete m_dtls;
  delete m_clientSocket;
  m_dtls = nullptr;
  m_clientSocket = nullptr;
}

HueStream::HueStream( QObject *parent) : QObject(parent), m_dtls(nullptr), m_clientSocket(nullptr)


{
}

void HueStream::connectStream(const QString &username, const QString &clientkey, const QString &huebridge) {
  m_username = username;
  m_clientkey = clientkey;
  QSslConfiguration config = QSslConfiguration::defaultDtlsConfiguration();
  config.setProtocol(QSsl::DtlsV1_2);
  config.setPeerVerifyMode(QSslSocket::VerifyNone);
  QString huecipher = "PSK-AES128-GCM-SHA256";
  QList<QSslCipher> ciphers;
  ciphers << QSslCipher(huecipher);
  config.setCiphers(ciphers);

  m_dtls = new QDtls(QSslSocket::SslClientMode);
  m_dtls->setPeer(QHostAddress(huebridge), 2100);
  m_dtls->setDtlsConfiguration(config);

  connect(m_dtls, &QDtls::pskRequired, this, &HueStream::onPskRequired);
  connect(m_dtls, &QDtls::handshakeTimeout, this, &HueStream::onHandshakeTimeout);

  m_clientSocket = new QUdpSocket;
  connect(m_clientSocket, &QUdpSocket::readyRead, this, &HueStream::onDataAvailable);
  m_clientSocket->connectToHost(QHostAddress(huebridge), 2100);

  if (!m_dtls->doHandshake(m_clientSocket)) {
    QString err = m_dtls->dtlsErrorString();
    qDebug() << "error" << err;
    emit statusChanged(err, true);
  }
}

void HueStream::onPskRequired(QSslPreSharedKeyAuthenticator *authenticator)
{
  authenticator->setIdentity(m_username.toLatin1());
  QByteArray arr = QByteArray::fromHex(m_clientkey.toLatin1());
  authenticator->setPreSharedKey(arr);
}

void HueStream::onHandshakeTimeout()
{
  if (!m_dtls->handleTimeout(m_clientSocket)) {
      qDebug() << "socket timeout fail";
      emit statusChanged("timeout", true);
      emit timeoutError();
  }
}


void HueStream::onDataAvailable()
{
  QByteArray dgram(m_clientSocket->pendingDatagramSize(), Qt::Uninitialized);
  qint64 bytes = m_clientSocket->readDatagram(dgram.data(), dgram.size());
  dgram.resize(bytes);
  if (m_dtls->isConnectionEncrypted()) {
    QByteArray data = m_dtls->decryptDatagram(m_clientSocket, dgram);
    qDebug() << "is encrypted";
    qDebug() << "todo do something with this";
    qDebug() << "data:" << data << bytes;
    qDebug() << "data:" << dgram;
  } else {
    if (!m_dtls->doHandshake(m_clientSocket, dgram)) {
      qDebug() << "error in continue handshake";
      qDebug() << m_dtls->dtlsErrorString();
    }
  }
  if (m_dtls->isConnectionEncrypted()) {
    qDebug() << "is encrypted";
    emit streamEstablished(true);
    emit statusChanged("active", false);
  }
}

bool HueStream::isReadyToStream() const
{
  return (m_dtls == nullptr || !m_dtls->isConnectionEncrypted());
}


void HueStream::send(const LightPacket &lp)
{
  if (!isReadyToStream()) {
    qDebug() << "encrypted connection not set up";
    return;
  }
  //qint64 written =
      m_dtls->writeDatagramEncrypted(m_clientSocket, lp);

}

