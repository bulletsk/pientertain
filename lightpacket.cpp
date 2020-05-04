#include "lightpacket.hh"
#include "colorconversion.hh"

#include <QDataStream>
#include <QDebug>
#include <QtEndian>

static const char *s_protocol = "HueStream";
static const int s_protocolLength = 9;
static const int s_maxNumberofLights = 10;
static const int s_headerSize = 16;
static const int s_lightdataSize = 9;

LightPacket::LightPacket(bool useRGB, bool useGammaCompensation)
  : QByteArray(s_headerSize, 0), m_sendRGB(useRGB), m_useGammaCompensation(useGammaCompensation)
{
  memcpy(data(), s_protocol, s_protocolLength);

  // version major
  char v = 0x01;
  memcpy(data()+9, &v, 1);

  char mode = m_sendRGB ? 0x00 : 0x01; // 0x01 xy brightness, 0x00 rgb
  memcpy(data()+14, &mode, 1);
}

void LightPacket::setSequenceNumber(int num) {
  char cNum = (num % 256);
  memcpy(data()+11, &cNum, 1);
}

void LightPacket::addLightData(const Light &light)
{

  if (maxLights() == numLights()) {
    qDebug() << "packet has max size";
    return;
  }

  QVector3D rgb = light.colorRGB();
  if (m_useGammaCompensation) {
    rgb = ColorConversion::GammaCorrection(rgb);
  }
  if (!m_sendRGB) {
    rgb = ColorConversion::RGBtoxyY(rgb, light.gamut());
  }
  const uint16_t id = static_cast<uint16_t>(light.id());
  const uint16_t r = static_cast<uint16_t>(rgb[0] * 65535.0f);
  const uint16_t g = static_cast<uint16_t>(rgb[1] * 65535.0f);
  const uint16_t b = static_cast<uint16_t>(rgb[2] * 65535.0f);

  QByteArray lightdata(s_lightdataSize,Qt::Uninitialized);
  char *data = lightdata.data();

  const uint8_t type = 0; // type light
  const uint16_t nId = qToBigEndian(id);
  const uint16_t nR = qToBigEndian(r);
  const uint16_t nG = qToBigEndian(g);
  const uint16_t nB = qToBigEndian(b);

  memcpy(data, &type, 1);
  memcpy(data+1, (const char*)&nId, 2);
  memcpy(data+3, (const char*)&nR, 2);
  memcpy(data+5, (const char*)&nG, 2);
  memcpy(data+7, (const char*)&nB, 2);

  append(lightdata);

}


int LightPacket::numLights() const
{
  return (size()-s_headerSize)/(s_lightdataSize);
}
int LightPacket::maxLights() const
{
  return s_maxNumberofLights;
}
