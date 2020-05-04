#ifndef LIGHT_HH
#define LIGHT_HH

#include <QVector3D>
#include <QVector2D>
#include <QJsonArray>

class Light {
public:

  struct Gamut {
    QVector2D red;
    QVector2D green;
    QVector2D blue;
  };

  Light(int id=-1);

  void setPosition(float x, float y, float z);

  void setColorGamutFromJSON( const QJsonArray &arr );
  void setColorRGB(float r, float g, float b);
  void setColorRGB(const QVector3D &rgb);

  int id() const;
  QVector3D pos() const;
  Gamut gamut() const;

  QVector3D colorRGB() const;
  QVector3D colorxyY(bool gammacorrected = false) const;

  void dump() const;

protected:
  int m_id;
  Gamut m_gamut;
  QVector3D m_rgb_color;
  QVector3D m_pos;
};

#endif // LIGHT_HH
