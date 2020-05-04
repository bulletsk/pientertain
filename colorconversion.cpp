#include <QVector2D>
#include "colorconversion.hh"

QVector3D ColorConversion::GammaCorrection(const QVector3D &rgb) {
  QVector3D ret;
  ret[0] = (rgb[0] > 0.04045f) ? pow((rgb[0] + 0.055f) / (1.0f + 0.055f), 2.4f) : (rgb[0] / 12.92f);
  ret[1] = (rgb[1] > 0.04045f) ? pow((rgb[1] + 0.055f) / (1.0f + 0.055f), 2.4f) : (rgb[1] / 12.92f);
  ret[2] = (rgb[2] > 0.04045f) ? pow((rgb[2] + 0.055f) / (1.0f + 0.055f), 2.4f) : (rgb[2] / 12.92f);
  return ret;
}


QVector3D ColorConversion::RGBtoxyY(const QVector3D &rgb, const Light::Gamut &gamut) {
  float red, green, blue;
  red = rgb[0];
  green = rgb[1];
  blue = rgb[2];

  float X = red * 0.649926f + green * 0.103455f + blue * 0.197109f;
  float Y = red * 0.234327f + green * 0.743075f + blue * 0.022598f;
  float Z = red * 0.0000000f + green * 0.053077f + blue * 1.035763f;

  float x = X / (X + Y + Z);
  float y = Y / (X + Y + Z);

  if (isInside( gamut, QVector2D(x,y))) {
    return QVector3D(x,y,Y);
  }

  QVector2D testPoint(x,y);
  // clip to gamut
  QVector2D bestPoint;
  float bestDistance = closestPointOnLine(gamut.red, gamut.green, testPoint, bestPoint);
  float distance;
  QVector2D curPoint;
  distance = closestPointOnLine(gamut.green, gamut.blue, testPoint, curPoint);
  if (distance < bestDistance) {
    bestDistance = distance;
    bestPoint = curPoint;
  }
  distance = closestPointOnLine(gamut.blue, gamut.red, testPoint, curPoint);
  if (distance < bestDistance) {
    bestDistance = distance;
    bestPoint = curPoint;
  }

  return QVector3D( bestPoint.x(), bestPoint.y(), Y );
}



bool ColorConversion::isInside( const Light::Gamut &gamut, const QVector2D & x ) {

  const QVector2D p0(gamut.green-gamut.red);
  const QVector2D p1(gamut.blue-gamut.red);
  const QVector2D p2(x-gamut.red);

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

float ColorConversion::closestPointOnLine( const QVector2D &p1, const QVector2D &p2, const QVector2D &x, QVector2D &closest) {

  const QVector2D unitDir = (p2-p1).normalized();
  const float dot = qBound(0.0f, QVector2D::dotProduct( x-p1, unitDir ), (p2-p1).length() );
  closest = p1 + dot * unitDir;
  return (x-closest).length();
}

