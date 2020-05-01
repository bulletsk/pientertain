#ifndef HUEAUTHENTICATION_H_
#define HUEAUTHENTICATION_H_

#include <QObject>
#include <QtNetwork>
#include <QVector>
#include "lightgroup.hh"

class HueAuthentication : public QObject
{
  Q_OBJECT
public:

  enum AuthState {

    NoClientKey=0,
    FindEntertainmentGroup,
    EnableStreaming,
    DisableStreaming,
    LastState
  };

  void startAuthentication();

  HueAuthentication();
  virtual ~HueAuthentication();

  QString userName() const;
  QString clientKey() const;

  LightGroup lightGroup(int number) const;

  void readSettings();
  void writeSettings();

signals:
  void streamingActive(bool on);
  void statusChanged(QString err, bool isError);

public slots:
  void onAuthenticationStateChange();
  void stopStreaming();

protected slots:

  void onDataAvailable();
  void onRequestFinished();

private:

  const QString m_appName;

  QString m_username;
  QString m_clientkey;

  QNetworkReply *m_reply;
  QString m_huebridgeIp;
  QUrl url;
  QNetworkAccessManager m_networkmanager;
  QByteArray m_response;

  AuthState m_currentState;

  QVector<LightGroup> m_groups;

};

#endif // HUEAUTHENTICATION_H_
