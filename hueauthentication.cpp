#include "hueauthentication.hh"

#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonObject>

const static QString s_defaultHueIp = "put bridge IP here";

HueAuthentication::HueAuthentication() : QObject(),
  m_appName("pientertain#"+QHostInfo::localHostName()), m_reply(nullptr), m_currentState(NoClientKey)
{
  readSettings();
}
HueAuthentication::~HueAuthentication()
{
  writeSettings();
}

bool HueAuthentication::isSetup() const {
  bool isIpv4Adress;
  QHostAddress(m_huebridgeIp).toIPv4Address(&isIpv4Adress);
  return isIpv4Adress;
}

QString HueAuthentication::userName() const {
  return m_username;
}
QString HueAuthentication::clientKey() const {
  return m_clientkey;
}
QString HueAuthentication::hueBridgeIp() const {
  return m_huebridgeIp;
}

LightGroup HueAuthentication::lightGroup(int number) const
{
  if (number<0 || number >= m_groups.size()) {
    return LightGroup();
  }
  return m_groups[number];
}


void HueAuthentication::startAuthentication()
{
  if (m_currentState == EnableStreaming) {
    // we are started already
    return;
  }
  if (m_clientkey.length() == 0) {
    m_currentState = NoClientKey;
  } else {
    m_currentState = FindEntertainmentGroup;
  }

  onAuthenticationStateChange();
}

void HueAuthentication::stopStreaming()
{
  if (m_currentState == DisableStreaming) {
    // we are stopped already
    return;
  }
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
    qDebug() << "enable streaming " << enable;
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
  default:
    break;
  }
}


void HueAuthentication::readSettings()
{
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
  settings.beginGroup("auth");
  m_username = settings.value("user", "").toString();
  m_clientkey = settings.value("key", "").toString();
  m_huebridgeIp = settings.value("bridgeIp", s_defaultHueIp).toString();
  m_appName = settings.value("appName", m_appName).toString();
  settings.endGroup();
}

void HueAuthentication::writeSettings()
{
  QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
  settings.beginGroup("auth");
  settings.setValue("user", m_username);
  settings.setValue("key", m_clientkey);
  settings.setValue("bridgeIp", m_huebridgeIp);
  settings.setValue("appName", m_appName);
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
    qDebug() << "error" << m_response;
    emit statusChanged(m_response, true);
    return;
  }

  delete m_reply;
  m_reply = nullptr;

  const QJsonDocument doc = QJsonDocument::fromJson(m_response);
  m_response.clear();

  switch( m_currentState ) {
  case NoClientKey:
  {
    const QJsonArray arr = doc.array();
    for (const QJsonValue &ref : arr) {
      const QJsonObject obj = ref.toObject();
      if (obj.contains("error")) {
        const QString err = obj["error"].toObject()["description"].toString();
        qDebug() << err;
        emit statusChanged(err, true);
        QTimer::singleShot(5000, this, &HueAuthentication::onAuthenticationStateChange);
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
    const QJsonObject obj = doc.object();
    const QStringList keys = obj.keys();
    for (const QString &key : keys) {
      int id = key.toInt();
      const LightGroup lg = LightGroup(id, obj[key].toObject() );
      if (lg.type() == LightGroup::Entertainment) {
        m_groups.append( lg );
      }
    }
    for (const LightGroup &lg : qAsConst(m_groups)) {
      lg.dump();
    }
    if (!m_groups.empty()) {
      m_currentState = EnableStreaming;
      QTimer::singleShot(500, this, &HueAuthentication::onAuthenticationStateChange);
    } else {
      emit statusChanged("no entertainment group", true);
    }
  }
    break;
  case EnableStreaming:
  case DisableStreaming:
  {
    const QJsonArray arr = doc.array();
    for (const QJsonValue &ref : arr) {
      const QJsonObject obj = ref.toObject();
      if (obj.contains("error")) {
        const QString err = obj["error"].toObject()["description"].toString();
        qDebug() << err;
        emit statusChanged(err, true);
      } else if (obj.contains("success")) {
        if (m_currentState == EnableStreaming) {
          emit streamingActive(true);
          emit statusChanged("stream enabled", false);
        } else {
          emit streamingActive(false);
          emit statusChanged("stream disabled", false);
        }
      }
    }
  }
    break;
  default:
    break;
  }
}
