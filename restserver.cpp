#include "restserver.hh"
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include <QImage>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>

const int s_serverPort = 8999;

RESTServer::RESTServer(QObject *parent) : QObject(parent), m_serverSocket(new QTcpServer), m_listenPort(s_serverPort),
  m_bridgeStatus("ok"), m_streamStatus("ok"), m_videoStatus("ok")
{
  m_corners.append(QPoint(0,0));

  readSettings();
}

RESTServer::~RESTServer()
{
  writeSettings();
  stopServer();
  delete m_serverSocket;
}


void RESTServer::readSettings()
{
  m_corners.clear();
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName());
  settings.beginGroup("servercorners");
  for (int i=1;i<=4;i++) {
    QPoint p = settings.value("point"+QString::number(i), QPoint(-1,-1)).toPoint();
    if ( p.x() < 0) {
      m_corners.clear();
      break;
    }
    m_corners.append(p);
  }
  settings.endGroup();
  QString json = settings.value("camsettings", "").toString();
  QJsonDocument doc = QJsonDocument::fromJson( json.toLatin1() );
  m_cameraSettings = doc.object();
  emit cameraSettingsChanged(m_cameraSettings);
}

void RESTServer::writeSettings()
{
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName());
  if (m_corners.size() == 4) {
    settings.beginGroup("servercorners");
    for (int i=1;i<=4;i++) {
      settings.setValue("point"+QString::number(i), m_corners[i-1]);
    }
    settings.endGroup();
  }
  if (!m_cameraSettings.empty()) {
    QJsonDocument doc(m_cameraSettings);
    QString val = doc.toJson(QJsonDocument::Compact);
    settings.setValue("camsettings", val);
  }
}




void RESTServer::startServer()
{
  m_serverSocket->listen(QHostAddress::Any, m_listenPort);
  connect(m_serverSocket, &QTcpServer::newConnection, this, &RESTServer::onNewConnection);
}

void RESTServer::stopServer()
{
  if (m_serverSocket->isListening()) {
    m_serverSocket->close();
  }
  QCoreApplication::exit();
}


void RESTServer::onNewConnection()
{
  QTcpSocket *clientSocket;
  while ((clientSocket = m_serverSocket->nextPendingConnection())!=nullptr) {
    connect( clientSocket, &QTcpSocket::readyRead, this, &RESTServer::handleRequest );
    connect( clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater );
  }
}


void RESTServer::handleRequest()
{
  QTcpSocket *clientSocket = (QTcpSocket *)sender();
  QByteArray request = clientSocket->readAll();

  // split into header and content
  int headerlen = request.indexOf("\r\n\r\n");
  if (headerlen < 0) {
    qDebug() << "invalid request";
    clientSocket->disconnectFromHost();
    return;
  }
  QString header(request.mid(0,headerlen));
  QByteArray content = request.mid(headerlen+2);

  qDebug() << header;

  QStringList headers = header.split("\r\n");
  if (headers.empty()) {
    qDebug() << "invalid request";
    clientSocket->disconnectFromHost();
    return;
  }

  QStringList reqString = headers[0].split(" ");
  if (reqString.size()!=3) {
    qDebug() << "invalid request";
    clientSocket->disconnectFromHost();
    return;
  }

  QString method, resource, version;
  method = reqString[0];
  resource = reqString[1];
  version = reqString[2];

  qDebug() << "X" << method << resource << version;

  if (method == "GET") {
    handleGet(clientSocket, resource);
  }

  if (method == "PUT") {
    handlePut(clientSocket, resource, content);
  }

  clientSocket->disconnectFromHost();
}


void RESTServer::onVideoImage(const QImage &image)
{
  m_latestImageJPG.clear();
  QBuffer qio(&m_latestImageJPG);
  image.save(&qio, "jpg", 10);
}

QJsonObject RESTServer::cameraSettings() const {
  return m_cameraSettings;
}


void RESTServer::onCameraSettingsChanged( const QJsonObject &json )
{
  m_cameraSettings = json;
}


void RESTServer::handleGet(QTcpSocket *socket, const QString &resource)
{
  QString mimetype = "application/json";
  if (resource == "/") {
    QString data = "[]";
    send(socket, data.toLatin1(), mimetype);
  } else if (resource == "/corners") {

    QJsonArray arr;
    for (const QPoint &p : qAsConst(m_corners)) {
      QJsonObject jp;
      jp["x"] = p.x();
      jp["y"] = p.y();
      arr.append(jp);
    }
    const QJsonDocument doc( arr );
    const QByteArray data = doc.toJson(QJsonDocument::Compact);
    send(socket, data, mimetype);
  } else if (resource == "/image") {
    emit requestImage();
    mimetype = "image/jpeg";
    if (m_latestImageJPG.isEmpty()) {
      QImage img(640,480, QImage::Format_RGB888);
      img.fill(0);
      QByteArray imagedata;
      QBuffer qio(&imagedata);
      img.save(&qio, "jpg", 10);
      send(socket, imagedata, mimetype);
    } else {
      send(socket, m_latestImageJPG, mimetype);
    }
  } else if (resource == "/status") {
    QJsonObject obj;
    obj["bridge"] = m_bridgeStatus;
    obj["stream"] = m_streamStatus;
    obj["video"] = m_videoStatus;
    obj["camerasettings"] = m_cameraSettings;
    const QJsonDocument doc( obj );
    const QByteArray data = doc.toJson(QJsonDocument::Compact);
    send(socket, data, mimetype);
  } else if (resource == "/start") {
    const QByteArray data = "{}";
    send(socket, data, mimetype);
    emit requestStart();
  } else if (resource == "/stop") {
    const QByteArray data = "{}";
    send(socket, data, mimetype);
    emit requestStop();
  } else if (resource == "/shutdown") {
    const QByteArray data = "{}";
    send(socket, data, mimetype);
    emit requestShutdown();
  } else {
    sendError(socket);
  }
}

void RESTServer::handlePut(QTcpSocket *socket, const QString &resource, const QByteArray &data)
{
  // todo validate input!!

  if (resource == "/corners") {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QVector<QPoint> points;
    const QJsonArray arr = doc.array();
    for (const QJsonValue &v : arr) {
      const QJsonObject o = v.toObject();
      QPoint p;
      p.setX( o["x"].toInt() );
      p.setY( o["y"].toInt() );
      points.append(p);
    }
    const QString mimetype = "application/json";
    const QByteArray reply = "{}";
    send(socket, reply, mimetype);
    emit cornersChanged( points );
    if (points.length()==4) {
      m_corners = points;
    }
  } else if (resource == "/camsettings") {
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    emit cameraSettingsChanged(doc.object());
  } else {
    sendError(socket);
  }
}

void RESTServer::send(QTcpSocket *socket, const QByteArray &data, const QString &mimetype) const
{
  QString header = ""
      "HTTP/1.1 200 OK\r\n"
      "Server: PiEntertain\r\n"
      "Content-Length: " + QString::number(data.length()) + "\r\n"
      "Connection: close\r\n"
      "Cache-Control: no-cache\r\n"
      "Vary: Accept-Encoding\r\n"
      "Content-Type: " + mimetype + "\r\n\r\n";
  QByteArray response = header.toLatin1();
  response.append(data);
  socket->write(response);
}

void RESTServer::sendError(QTcpSocket *socket) const
{
  // todo
  Q_UNUSED(socket);
}


void RESTServer::onBridgeStatus(const QString &status, bool err)
{
  m_bridgeStatus = (err ? "E " : "S ") + status;
}

void RESTServer::onStreamStatus(const QString &status, bool err)
{
  m_streamStatus = (err ? "E " : "S ") + status;
}
void RESTServer::onVideoStatus(const QString &status, bool err)
{
  m_videoStatus = (err ? "E " : "S ") + status;
}

