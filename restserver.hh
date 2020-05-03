#ifndef RESTSERVER_HH
#define RESTSERVER_HH

#include <QTcpServer>
#include <QJsonObject>

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

  QJsonObject cameraSettings() const;


signals:

  void requestStart();
  void requestStop();
  void requestShutdown();
  void requestImage();
  void cornersChanged( const QVector<QPoint>& );
  void cameraSettingsChanged( const QJsonObject &camSettingsJSON );

public slots:
  // put status messages here
  void onBridgeStatus(const QString &status, bool err);
  void onStreamStatus(const QString &status, bool err);
  void onVideoStatus(const QString &status, bool err);
  void onVideoImage(const QImage &image);
  void onCameraSettingsChanged(const QJsonObject &json );
  void onCornersChanged( const QVector<QPoint> &corners );

protected slots:
  void onNewConnection();
  void handleRequest();

protected:
  void handleGet(QTcpSocket *socket, const QString &resource);
  void handlePut(QTcpSocket *socket, const QString &resource, const QByteArray &data);

  void send(QTcpSocket *socket, const QByteArray &data, QString const &mimetype) const;
  void sendError(QTcpSocket *socket) const;

private:
  QTcpServer *m_serverSocket;

  int m_listenPort;

  QString m_bridgeStatus;
  QString m_streamStatus;
  QString m_videoStatus;

  QJsonObject m_cameraSettings;

  QByteArray m_latestImageJPG;

  QVector<QPoint> m_corners;



};

#endif // RESTSERVER_HH
