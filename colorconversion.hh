#ifndef COLORCONVERSION_HH
#define COLORCONVERSION_HH

#include <QVector3D>
#include "light.hh"

/*
 * https://github.com/johnciech/PhilipsHueSDK/blob/master/ApplicationDesignNotes/RGB%20to%20xy%20Color%20conversion.md
 */

class ColorConversion
{
public:
  static QVector3D GammaCorrection(const QVector3D &rgb);
  static QVector3D RGBtoxyY(const QVector3D &rgb, const Light::Gamut &gamut);

private:

  static bool isInside( const Light::Gamut &gamut, const QVector2D & x );
  static float closestPointOnLine( const QVector2D &p1, const QVector2D &p2, const QVector2D &x, QVector2D &closest);


};

#endif // COLORCONVERSION_HH
