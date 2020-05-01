#ifndef LIGHTGROUP_HH
#define LIGHTGROUP_HH

#include <QJsonObject>
#include <QVector3D>
#include <QVector>

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

  struct Light {
    int id;
    QVector3D pos;
  };

  LightGroup();
  LightGroup(int id, const QJsonObject &obj);

  int id() const;

  LightGroup::Type type() const;
  QString name() const;
  QString entertainmentClass() const;

  QVector<Light> lights() const;

  void dump() const;

protected:
  int m_id;
  QVector<Light> m_lights;
};

#endif // LIGHTGROUP_HH
