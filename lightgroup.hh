#ifndef LIGHTGROUP_HH
#define LIGHTGROUP_HH

#include <QJsonObject>
#include <QVector3D>
#include <QVector2D>
#include <QVector>
#include <QList>

class Light {
public:
  Light(int _id=-1) : id(_id), pos(0.0f,0.0f,0.0f),
    m_gamut_red(1.0f,0.0f),
    m_gamut_green(0.0f,1.0f),
    m_gamut_blue(0.0f,0.0f) {}

  void setColorGamutFromJSON( const QJsonArray &arr );
  QVector3D convertToxyY(QVector3D rgb, bool withGammaCompensation) const;

  void dump() const;

public:
  int id;
  QVector3D pos;
protected:
  QVector2D m_gamut_red;
  QVector2D m_gamut_green;
  QVector2D m_gamut_blue;
};

class LightGroup : public QJsonObject
{
public:

  enum Type {
    Room = 0,
    Entertainment,
    Group,
    Unknown,
    Invalid
  };

  LightGroup();
  LightGroup(int id, const QJsonObject &obj);

  int id() const;

  LightGroup::Type type() const;
  QString name() const;
  QString entertainmentClass() const;

  QVector<Light> lights() const;
  void setColorGamutsFromJSON( const QJsonObject &obj );

  void dump() const;

protected:

  QJsonValue getPath( const QJsonObject &obj, const QStringList &path );

  int m_id;
  QVector<Light> m_lights;
};

#endif // LIGHTGROUP_HH
