#include "lightpacket.hh"
#include <QDataStream>
#include <QDebug>
#include <QtEndian>

static const char *s_protocol = "HueStream";
static const int s_protocolLength = 9;
static const int s_maxNumberofLights = 10;
static const int s_headerSize = 16;
static const int s_lightdataSize = 9;

LightPacket::LightPacket(bool useRGB) : QByteArray(s_headerSize, 0)
{
  memcpy(data(), s_protocol, s_protocolLength);

  // version major
  char v = 0x01;
  memcpy(data()+9, &v, 1);

  char mode = !useRGB ? 0x00 : 0x01; // 0x01 xy brightness, 0x00 rgb
  memcpy(data()+14, &mode, 1);
}

void LightPacket::setSequenceNumber(int num) {
  char cNum = (num % 256);
  memcpy(data()+11, &cNum, 1);
}

void LightPacket::addLightData(const uint16_t id, const uint16_t r, const uint16_t g, const uint16_t b)
{

  if (maxLights() == numLights()) {
    qDebug() << "packet has max size";
    return;
  }

  QByteArray lightdata(s_lightdataSize,Qt::Uninitialized);
  char *data = lightdata.data();

  char type = 0; // type light
  unsigned short nId = qToBigEndian(id);
  unsigned short nR = qToBigEndian(r);
  unsigned short nG = qToBigEndian(g);
  unsigned short nB = qToBigEndian(b);

  nR = 0xffff;
  nG = 0xffff;
  nB = 0xffff;

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
