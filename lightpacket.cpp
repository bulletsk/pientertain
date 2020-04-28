#include "lightpacket.hh"
#include <QDataStream>
#include <QDebug>
#include <QtEndian>

LightPacket::LightPacket()
{
  resize(16);
  fill(0);

  const char *protocol = "HueStream";
  memcpy(data(), protocol, 9);

  char version[2] = { 0x01, 0x00 };
  memcpy(data()+9, version, 2);

  char mode = 0x00; // 0x01 xy brightness, 0x00 rgb
  memcpy(data()+14, &mode, 1);

}

void LightPacket::setSequenceNumber(int num) {
  char cNum = (num % 256);
  memcpy(data()+11, &cNum, 1);
}

void LightPacket::addLightData(unsigned short id, unsigned short r, unsigned short g, unsigned short b)
{

  if (maxLights() == numLights()) {
    qDebug() << "packet has max size";
    return;
  }
  QByteArray lightdata(9,Qt::Uninitialized);
  char *data = lightdata.data();

  char type = 0; // type light
  unsigned short nId = qToBigEndian(id);
  unsigned short nR = qToBigEndian(r);
  unsigned short nG = qToBigEndian(g);
  unsigned short nB = qToBigEndian(b);

  memcpy(data, &type, 1);
  memcpy(data+1, (const char*)&nId, 2);
  memcpy(data+3, (const char*)&nR, 2);
  memcpy(data+5, (const char*)&nG, 2);
  memcpy(data+7, (const char*)&nB, 2);

  append(lightdata);

}

//void LightPacket::addLightData(unsigned short id, unsigned short x, unsigned short y, unsigned short brightness)
//{

//  if (maxLights() == numLights()) {
//    qDebug() << "packet has max size";
//    return;
//  }
//  QByteArray lightdata(9,Qt::Uninitialized);
//  char *data = lightdata.data();

//  char type = 0; // type light
//  unsigned short nId = qToBigEndian(id);
//  unsigned short nX = qToBigEndian(x);
//  unsigned short nY = qToBigEndian(y);
//  unsigned short nB = qToBigEndian(brightness);

//  memcpy(data, &type, 1);
//  memcpy(data+1, (const char*)&nId, 2);
//  memcpy(data+3, (const char*)&nX, 2);
//  memcpy(data+5, (const char*)&nY, 2);
//  memcpy(data+7, (const char*)&nB, 2);

//  append(lightdata);

//}


int LightPacket::numLights() const
{
  return (size()-16)/9;
}
int LightPacket::maxLights() const
{
  return 10;
}
