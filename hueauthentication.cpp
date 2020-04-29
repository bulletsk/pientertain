#include "hueauthentication.hh"

#include <QSettings>
#include <QDebug>
#include <QJsonObject>

HueAuthentication::HueAuthentication() : QObject(), m_appName("pientertain#valhalla"), m_huebridgeIp("192.168.2.101"), m_currentState(NoClientKey)
{
  readSettings();

  m_username = "aSovHP9X3j7bzRD7Foxu90xBIEJHL0l99Sbtg6Q6";
  m_clientkey = "B68FD5B0E4AD16334303AE8A0D6CD403";

  qDebug() << m_username;
}
HueAuthentication::~HueAuthentication()
{
  writeSettings();
}

QString HueAuthentication::userName() const {
  return m_username;
}
QString HueAuthentication::clientKey() const {
  return m_clientkey;
}

LightGroup HueAuthentication::lightGroup(int number) const
{
  return m_groups[number];
}


void HueAuthentication::startAuthentication()
{
  if (m_clientkey.length() == 0) {
    m_currentState = NoClientKey;
  } else {
    m_currentState = FindEntertainmentGroup;
  }

  onAuthenticationStateChange();
}

void HueAuthentication::stopStreaming()
{
  m_currentState = DisableStreaming;
  onAuthenticationStateChange();
}

void HueAuthentication::onAuthenticationStateChange()
{
  QString surl = "http://"+m_huebridgeIp;

  switch (m_currentState) {
  case NoClientKey:
  {
    QJsonDocument doc;
    QJsonObject genkey;
    genkey["devicetype"] = m_appName;
    genkey["generateclientkey"] = true;
    doc.setObject(genkey);

    QNetworkRequest request( QUrl(surl+"/api") );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_reply = m_networkmanager.post(request, doc.toJson(QJsonDocument::Compact));
    connect(m_reply, &QNetworkReply::finished, this, &HueAuthentication::onRequestFinished);
    connect(m_reply, &QIODevice::readyRead, this, &HueAuthentication::onDataAvailable);
  }
    break;
  case FindEntertainmentGroup:
  {
    QNetworkRequest request( QUrl(surl+"/api/"+m_username+"/groups") );
    m_reply = m_networkmanager.get(request);
    connect(m_reply, &QNetworkReply::finished, this, &HueAuthentication::onRequestFinished);
    connect(m_reply, &QIODevice::readyRead, this, &HueAuthentication::onDataAvailable);
  }
    break;
  case EnableStreaming:
  case DisableStreaming:
  {
    bool enable = m_currentState == EnableStreaming;
    QJsonDocument doc;
    QJsonObject stream;
    QJsonObject active;
    active["active"] = enable;
    stream["stream"] = active;
    doc.setObject(stream);
    QNetworkRequest request( QUrl(surl+"/api/"+m_username+"/groups/"+QString::number(m_groups[0].id())) );
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_reply = m_networkmanager.put(request, doc.toJson(QJsonDocument::Compact));
    connect(m_reply, &QNetworkReply::finished, this, &HueAuthentication::onRequestFinished);
    connect(m_reply, &QIODevice::readyRead, this, &HueAuthentication::onDataAvailable);
  }
    break;
  }
}


void HueAuthentication::readSettings()
{
  QSettings settings(QSettings::UserScope);
  settings.beginGroup("auth");
  m_username = settings.value("user", "").toString();
  m_clientkey = settings.value("key", "").toString();
  settings.endGroup();
}

void HueAuthentication::writeSettings()
{
  QSettings settings(QSettings::UserScope);
  settings.beginGroup("auth");
  settings.setValue("user", m_username);
  settings.setValue("key", m_clientkey);
  settings.endGroup();
}

void HueAuthentication::onDataAvailable()
{
  qDebug() << "data available";
  m_response.append(m_reply->readAll());
}

void HueAuthentication::onRequestFinished()
{
  qDebug() << "request finished";

  bool error = m_reply->error();

  if (error) {
    qDebug() << "error";
    qDebug() << m_response;
    emit onConnectionError(m_response);
    return;
  }

  delete m_reply;
  m_reply = nullptr;

  QJsonDocument doc = QJsonDocument::fromJson(m_response);
  m_response.clear();

  bool exit = true;

  switch( m_currentState ) {
  case NoClientKey:
  {
    QJsonArray arr = doc.array();
    for (QJsonValueRef ref : arr) {
      QJsonObject obj = ref.toObject();
      if (obj.contains("error")) {
        qDebug() << obj["error"].toObject()["description"].toString();
        QTimer::singleShot(5000, this, &HueAuthentication::onAuthenticationStateChange);
        exit = false;
      } else if (obj.contains("success")) {
        qDebug() << doc.toJson(QJsonDocument::Compact);
        m_username = obj["success"].toObject()["username"].toString();
        m_clientkey = obj["success"].toObject()["clientkey"].toString();
        m_currentState = FindEntertainmentGroup;
      }
    }
  }
    break;
  case FindEntertainmentGroup:
  {
    m_groups.clear();
    QJsonObject obj = doc.object();
    for (QString key : obj.keys()) {
      int id = key.toInt();
      LightGroup lg = LightGroup(id, obj[key].toObject() );
      if (lg.type() == LightGroup::Entertainment) {
        m_groups.append( lg );
      }
    }
//    qDebug().noquote() << doc.toJson();

    for (LightGroup &lg : m_groups) {
      lg.dump();
    }
    if (!m_groups.empty()) {
      exit = false;
      m_currentState = EnableStreaming;
      QTimer::singleShot(500, this, &HueAuthentication::onAuthenticationStateChange);
    }
  }
    break;
  case EnableStreaming:
  case DisableStreaming:
  {
    QJsonArray arr = doc.array();
    for (QJsonValueRef ref : arr) {
      QJsonObject obj = ref.toObject();
      if (obj.contains("error")) {
        qDebug() << obj["error"].toObject()["description"].toString();
        exit = false;
      } else if (obj.contains("success")) {
        qDebug() << doc.toJson(QJsonDocument::Compact);
        if (m_currentState == EnableStreaming) {
          m_currentState = DisableStreaming;
          exit = false;
          emit streamingActive(true);
        } else {
          // go to exit
          emit streamingActive(false);
        }
      }
    }
  }
    break;
  }
}
