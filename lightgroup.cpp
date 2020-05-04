#include "lightgroup.hh"
#include <QDebug>
#include <QJsonArray>
#include <QVector2D>

LightGroup::LightGroup() : m_id(-1){}

LightGroup::LightGroup(int id, const QJsonObject &obj) : QJsonObject(obj), m_id(id)
{
  if (contains("lights")) {
    const QJsonArray arr = value("lights").toArray();
    for (const QJsonValue &ref : arr) {
      const QString sid = ref.toString();
      m_lights.append(Light(sid.toInt()));
    }
  }
  if (contains("locations")) {
    QJsonObject locations = value("locations").toObject();
    for (Light &light : m_lights) {
      QString key = QString::number(light.id());
      if (!locations.contains(key)) {
        continue;
      }
      QJsonArray arr = locations[key].toArray();
      if (arr.size()<3) {
        qDebug() << "location can not be parsed";
        continue;
      }
      light.setPosition( static_cast<float>(arr[0].toDouble()),
                         static_cast<float>(arr[1].toDouble()),
                         static_cast<float>(arr[2].toDouble()));
    }
  }
}

int LightGroup::id() const
{
  return m_id;
}

LightGroup::Type LightGroup::type() const
{
  if (!contains("type")) {
    return Invalid;
  }

  QString type = value("type").toString();

  if (type == "Entertainment") {
    return Entertainment;
  }

  if (type == "Room") {
    return Room;
  }

  if (type == "LightGroup") {
    return Group;
  }

  qDebug() << "unknown group type" << type << "for id" << id();
  return Unknown;

}

QString LightGroup::name() const
{
  if (!contains("name")) {
    return "no name";
  }
  return value("name").toString();
}

QString LightGroup::entertainmentClass() const
{
  if (!contains("class")) {
    return "no class";
  }
  return value("class").toString();
}

QVector<Light> LightGroup::lights() const
{
  return m_lights;
}

void LightGroup::setColorGamutsFromJSON( const QJsonObject &obj )
{
  for (Light &light : m_lights) {
    QString id = QString::number( light.id());
    QStringList path;
    path << id << "capabilities" << "control" << "colorgamut";
    QJsonValue gamut = getPath(obj, path);
    if (!gamut.isArray()) continue;
    QJsonArray arr = gamut.toArray();
    light.setColorGamutFromJSON(arr);
  }
}



void LightGroup::dump() const
{
  qDebug() << "group" << id() << "type" << type() << "name" << name() << "class" << entertainmentClass();
  qDebug() << "lights";
  for (const Light &l : m_lights) {
    qDebug() << l.id() << l.pos();
    l.dump();
  }
}


QJsonValue LightGroup::getPath( const QJsonObject &obj, const QStringList &path )
{
  QJsonValue curValue = obj;
  for (const QString &cur : path) {
    QJsonObject o = curValue.toObject();
    if (o.contains(cur)) {
      curValue = o[cur];
    } else {
      return QJsonValue();
    }
  }
  return curValue;
}

