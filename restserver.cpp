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

RESTServer::RESTServer(QObject *parent) : QObject(parent), m_serverSocket(new QTcpServer), m_listenPort(8999)
{

  m_bridgeStatus = "ok";
  m_streamStatus = "ok";
  m_videoStatus = "ok";

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
  if (m_corners.size() != 4) {
    return;
  }
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName());
  settings.beginGroup("servercorners");
  for (int i=1;i<=4;i++) {
    settings.setValue("point"+QString::number(i), m_corners[i-1]);
  }
  settings.endGroup();
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

void RESTServer::onCameraSettingsChanged( QJsonObject json )
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
    for (QPoint p : m_corners) {
      QJsonObject jp;
      jp["x"] = p.x();
      jp["y"] = p.y();
      arr.append(jp);
    }
    QJsonDocument doc( arr );
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    send(socket, data, mimetype);
  } else if (resource == "/image") {
    emit requestImage();
    mimetype = "image/jpeg";
    if (m_latestImageJPG.isEmpty()) {
      QImage img(1920,1080, QImage::Format_RGB32);
      img.fill(0);
      QByteArray imagedata;
      QBuffer qio(&imagedata);
      img.save(&qio, "jpg", 10);
    }
    send(socket, m_latestImageJPG, mimetype);
  } else if (resource == "/status") {
    QJsonObject obj;
    obj["bridge"] = m_bridgeStatus;
    obj["stream"] = m_streamStatus;
    obj["video"] = m_videoStatus;
    obj["camerasettings"] = m_cameraSettings;
    QJsonDocument doc( obj );
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    send(socket, data, mimetype);
  } else if (resource == "/start") {
    QJsonObject obj;
    QJsonDocument doc( obj );
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    send(socket, data, mimetype);
    emit requestStart();
  } else if (resource == "/stop") {
    QJsonObject obj;
    QJsonDocument doc( obj );
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    send(socket, data, mimetype);
    emit requestStop();
  } else if (resource == "/shutdown") {
    QJsonObject obj;
    QJsonDocument doc( obj );
    QByteArray data = doc.toJson(QJsonDocument::Compact);
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
    QJsonArray arr = doc.array();
    for (QJsonValueRef v : arr) {
      QJsonObject o = v.toObject();
      QPoint p;
      p.setX( o["x"].toInt() );
      p.setY( o["y"].toInt() );
      points.append(p);
    }
    QString mimetype = "application/json;charset=utf-8";
    QJsonObject obj;
    QJsonDocument docR( obj );
    QByteArray reply = docR.toJson(QJsonDocument::Compact);
    send(socket, reply, mimetype);
    emit cornersChanged( points );
    if (points.length()==4) {
      m_corners = points;
    }
  } else if (resource == "/camsettings") {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    emit cameraSettingsChanged(doc.object());
  } else {
    sendError(socket);
  }
}

void RESTServer::send(QTcpSocket *socket, const QByteArray &data, QString &mimetype)
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

void RESTServer::sendError(QTcpSocket *socket) {

}


void RESTServer::onBridgeStatus(QString status, bool err)
{
  m_bridgeStatus = (err ? "E " : "S ") + status;
}

void RESTServer::onStreamStatus(QString status, bool err)
{
  m_streamStatus = (err ? "E " : "S ") + status;
}
void RESTServer::onVideoStatus(QString status, bool err)
{
  m_videoStatus = (err ? "E " : "S ") + status;
}

