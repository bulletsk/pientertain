#ifndef RESTSERVER_HH
#define RESTSERVER_HH

#include <QTcpServer>

class RESTServer : public QObject
{
  Q_OBJECT
public:
  explicit RESTServer(QObject *parent = nullptr);
  virtual ~RESTServer();
  void startServer();
  void stopServer();

  void readSettings();
  void writeSettings();


signals:

  void requestStart();
  void requestStop();
  void requestShutdown();
  void requestImage();
  void cornersChanged( QVector<QPoint> );

public slots:
  // put status messages here
  void onBridgeStatus(QString status, bool err);
  void onStreamStatus(QString status, bool err);
  void onVideoStatus(QString status, bool err);
  void onVideoImage(const QImage &image);

protected slots:
  void onNewConnection();
  void handleRequest();

protected:
  void handleGet(QTcpSocket *socket, const QString &resource);
  void handlePut(QTcpSocket *socket, const QString &resource, const QByteArray &data);

  void send(QTcpSocket *socket, const QByteArray &data, QString &mimetype);
  void sendError(QTcpSocket *socket);

private:
  QTcpServer *m_serverSocket;

  const int m_listenPort;

  QString m_bridgeStatus;
  QString m_streamStatus;
  QString m_videoStatus;

  QByteArray m_latestImageJPG;

  QVector<QPoint> m_corners;


};

#endif // RESTSERVER_HH
