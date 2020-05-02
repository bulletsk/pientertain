#include "lightgroup.hh"
#include <QDebug>
#include <QJsonArray>

LightGroup::LightGroup() : m_id(-1){}

LightGroup::LightGroup(int id, const QJsonObject &obj) : QJsonObject(obj), m_id(id)
{
  if (contains("lights")) {
    QJsonArray arr = value("lights").toArray();
    for (QJsonValueRef ref : arr) {
      QString sid = ref.toString();
      m_lights.append( { sid.toInt(), QVector3D(0.0,0.0,0.0) } );
    }
  }
  if (contains("locations")) {
    QJsonObject locations = value("locations").toObject();
    for (Light &light : m_lights) {
      QString key = QString::number(light.id);
      if (!locations.contains(key)) {
        continue;
      }
      QJsonArray arr = locations[key].toArray();
      if (arr.size()<3) {
        qDebug() << "location can not be parsed";
        continue;
      }
      light.pos.setX( arr[0].toDouble() );
      light.pos.setY( arr[1].toDouble() );
      light.pos.setZ( arr[2].toDouble() );

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


void LightGroup::dump() const
{
  qDebug() << "group" << id() << "type" << type() << "name" << name() << "class" << entertainmentClass();
  qDebug() << "lights";
  for (const Light &l : m_lights) {
    qDebug() << l.id << l.pos;
  }
}


