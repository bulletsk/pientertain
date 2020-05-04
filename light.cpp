#include "light.hh"
#include <QDebug>

static bool isInside( const QVector2D &red, const QVector2D &green, const QVector2D &blue, const QVector2D & x );
static float closestPointOnLine( const QVector2D &p1, const QVector2D &p2, const QVector2D &x, QVector2D &closest);

Light::Light(int id) : m_id(id), m_pos(0.0f,0.0f,0.0f)
{
  m_gamut.red = { 1.0f,0.0f };
  m_gamut.green = { 0.0f,1.0f };
  m_gamut.blue = { 0.0f,0.0f };
}


void Light::setColorGamutFromJSON( const QJsonArray &arr )
{
  if (arr.size() != 3) return;
  QJsonArray red = arr[0].toArray();
  QJsonArray green = arr[1].toArray();
  QJsonArray blue = arr[2].toArray();
  if (red.size()==2) {
    m_gamut.red = QVector2D((float)red[0].toDouble(), (float)red[1].toDouble());
  }
  if (green.size()==2) {
    m_gamut.green = QVector2D((float)green[0].toDouble(), (float)green[1].toDouble());
  }
  if (blue.size()==2) {
    m_gamut.blue = QVector2D((float)blue[0].toDouble(), (float)blue[1].toDouble());
  }
}

void Light::setPosition(float x, float y, float z)
{
  m_pos[0] = x;
  m_pos[1] = y;
  m_pos[2] = z;
}


void Light::setColorRGB(float r, float g, float b)
{
  m_rgb_color[0] = r;
  m_rgb_color[1] = g;
  m_rgb_color[2] = b;
}

void Light::setColorRGB(const QVector3D &rgb)
{
  m_rgb_color = rgb;
}

int Light::id() const
{
  return m_id;
}
QVector3D Light::pos() const
{
  return m_pos;
}

Light::Gamut Light::gamut() const
{
  return m_gamut;
}

void Light::dump() const
{
  qDebug() << "gamut" << m_gamut.red << m_gamut.green << m_gamut.blue;
}

QVector3D Light::colorRGB() const
{
  return m_rgb_color;
}

