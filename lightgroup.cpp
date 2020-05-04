#include "lightgroup.hh"
#include <QDebug>
#include <QJsonArray>
#include <QVector2D>

void Light::setColorGamutFromJSON( const QJsonArray &arr )
{
  if (arr.size() != 3) return;
  QJsonArray red = arr[0].toArray();
  QJsonArray green = arr[1].toArray();
  QJsonArray blue = arr[2].toArray();
  if (red.size()==2) {
    m_gamut_red = QVector2D((float)red[0].toDouble(), (float)red[1].toDouble());
  }
  if (green.size()==2) {
    m_gamut_green = QVector2D((float)green[0].toDouble(), (float)green[1].toDouble());
  }
  if (blue.size()==2) {
    m_gamut_blue = QVector2D((float)blue[0].toDouble(), (float)blue[1].toDouble());
  }
}


static bool isInside( const QVector2D &red, const QVector2D &green, const QVector2D &blue, const QVector2D & x ) {

  const QVector2D p0(green-red);
  const QVector2D p1(blue-red);
  const QVector2D p2(x-red);

  const float d00 = QVector2D::dotProduct(p0,p0);
  const float d01 = QVector2D::dotProduct(p0,p1);
  const float d02 = QVector2D::dotProduct(p0,p2);
  const float d11 = QVector2D::dotProduct(p1,p1);
  const float d12 = QVector2D::dotProduct(p1,p2);

  const float div = 1.0f / (d00*d11-d01*d01);
  const float s = (d11*d02 - d01*d12) * div;
  const float t = (d00*d12 - d01*d02) * div;
  return (s >= 0) && (t >= 0) && (s + t <= 1);
}

static float closestPointOnLine( const QVector2D &p1, const QVector2D &p2, const QVector2D &x, QVector2D &closest) {

  const QVector2D unitDir = (p2-p1).normalized();
  const float dot = qBound(0.0f, QVector2D::dotProduct( x-p1, unitDir ), 1.0f );
  closest = p1 + dot * unitDir;
  return (x-closest).length();
}


QVector3D Light::convertToxyY(QVector3D rgb, bool withGammaCompensation) const
{
  /*
   * https://github.com/johnciech/PhilipsHueSDK/blob/master/ApplicationDesignNotes/RGB%20to%20xy%20Color%20conversion.md
   */
  float red, green, blue;
  red = rgb[0];
  green = rgb[1];
  blue = rgb[2];
  if (withGammaCompensation)
  {
    red = (red > 0.04045f) ? pow((red + 0.055f) / (1.0f + 0.055f), 2.4f) : (red / 12.92f);
    green = (green > 0.04045f) ? pow((green + 0.055f) / (1.0f + 0.055f), 2.4f) : (green / 12.92f);
    blue = (blue > 0.04045f) ? pow((blue + 0.055f) / (1.0f + 0.055f), 2.4f) : (blue / 12.92f);
  }

  float X = red * 0.649926f + green * 0.103455f + blue * 0.197109f;
  float Y = red * 0.234327f + green * 0.743075f + blue * 0.022598f;
  float Z = red * 0.0000000f + green * 0.053077f + blue * 1.035763f;

  float x = X / (X + Y + Z);
  float y = Y / (X + Y + Z);

  if (isInside( m_gamut_red, m_gamut_green, m_gamut_blue, QVector2D(x,y))) {
    return QVector3D(x,y,Y);
  }

  QVector2D testPoint(x,y);
  // clip to gamut
  QVector2D bestPoint;
  float bestDistance = closestPointOnLine(m_gamut_red, m_gamut_green, testPoint, bestPoint);
  float distance;
  QVector2D curPoint;
  distance = closestPointOnLine(m_gamut_green, m_gamut_blue, testPoint, curPoint);
  if (distance < bestDistance) {
    bestDistance = distance;
    bestPoint = curPoint;
  }
  distance = closestPointOnLine(m_gamut_blue, m_gamut_red, testPoint, curPoint);
  if (distance < bestDistance) {
    bestDistance = distance;
    bestPoint = curPoint;
  }

  return QVector3D( bestPoint.x(), bestPoint.y(), Y );
}

void Light::dump() const
{
  qDebug() << "gamut" << m_gamut_red << m_gamut_green << m_gamut_blue;
}


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
      QString key = QString::number(light.id);
      if (!locations.contains(key)) {
        continue;
      }
      QJsonArray arr = locations[key].toArray();
      if (arr.size()<3) {
        qDebug() << "location can not be parsed";
        continue;
      }
      light.pos.setX( static_cast<float>(arr[0].toDouble()) );
      light.pos.setY( static_cast<float>(arr[1].toDouble()) );
      light.pos.setZ( static_cast<float>(arr[2].toDouble()) );

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
    QString id = QString::number( light.id );
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
    qDebug() << l.id << l.pos;
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

