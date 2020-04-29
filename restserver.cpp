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


RESTServer::RESTServer(QObject *parent) : QObject(parent), m_serverSocket(new QTcpServer), m_listenPort(8999)
{



}

RESTServer::~RESTServer()
{
  stopServer();
  delete m_serverSocket;
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


void RESTServer::handleGet(QTcpSocket *socket, const QString &resource)
{
  QString mimetype = "application/json";
  if (resource == "/") {
    QString data = "[]";
    send(socket, data.toLatin1(), mimetype);
  } else if (resource == "/corners") {
    /*
     * todo get from videosource
     *
     * /
     */
    QVector<QPoint> corners; // = videosource->corners
    corners.append(QPoint(0,0));
    corners.append(QPoint(123,234));

    QJsonArray arr;
    for (QPoint p : corners) {
      QJsonObject jp;
      jp["x"] = p.x();
      jp["y"] = p.y();
      arr.append(jp);
    }
    QJsonDocument doc( arr );
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    send(socket, data, mimetype);
  } else if (resource == "/image") {
    /*
     * todo get from videosource
     *
     * /
     */
    mimetype = "image/jpeg";
    QString imgName = QCoreApplication::applicationDirPath() + "/colortestimage.png";
    QImage img; //= videosource->image
    img.load(imgName);
    QByteArray imagedata;
    QBuffer qio(&imagedata);
    img.save(&qio, "jpg", 10);
    send(socket, imagedata, mimetype);
  } else if (resource == "/status") {
    /*
     * todo get from videosource
     *
     * /
     */
    QJsonObject obj;
    obj["bridge"] = "okay"; // todo get from hueauth
    obj["stream"] = "okay"; // todo get huestream
    obj["video"] = "okay"; // todo get from videosource

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

}

void RESTServer::send(QTcpSocket *socket, const QByteArray &data, QString &mimetype)
{
  QString header = ""
      "HTTP/1.1 200 OK\r\n"
      "Server: PiEntertain\r\n"
      "Content-Length: " + QString::number(data.length()) + "\r\n"
      "Connection: close"
      "Content-Type: " + mimetype + "\r\n\r\n";
  QByteArray response = header.toLatin1();
  response.append(data);
  socket->write(response);
}

void RESTServer::sendError(QTcpSocket *socket) {

}


