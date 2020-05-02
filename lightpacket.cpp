#include "lightpacket.hh"
#include <QDataStream>
#include <QDebug>
#include <QtEndian>

static const char *s_protocol = "HueStream";
static const int s_protocolLength = 9;
static const int s_maxNumberofLights = 10;

LightPacket::LightPacket() : QByteArray(sizeof(PacketHeader), 0)
{
  memcpy(data(), s_protocol, s_protocolLength);
  PacketHeader *header = (PacketHeader*)(data());

  header->versionMajor = 0x01;
//  header->versionMinor = 0x00;
//  header->colorSpace = 0x00; // 0x01 xy brightness, 0x00 rgb
}

void LightPacket::setSequenceNumber(int num) {
  char cNum = (num % 256);
  PacketHeader *header = (PacketHeader*)(data());
  header->sequenceNumber = cNum;
}

void LightPacket::addLightData(const uint16_t id, const uint16_t r, const uint16_t g, const uint16_t b)
{

  if (maxLights() == numLights()) {
    qDebug() << "packet has max size";
    return;
  }
  QByteArray lightdata(1+sizeof(LightData),Qt::Uninitialized);
  memset(lightdata.data(), 0x00, 1); // type is light
  LightData *data = (LightData*)(lightdata.data()+1);
  data->id = qToBigEndian(id);
  data->r = qToBigEndian(r);
  data->g = qToBigEndian(g);
  data->b = qToBigEndian(b);

  append(lightdata);

}

//void LightPacket::addLightData(unsigned short id, unsigned short x, unsigned short y, unsigned short brightness)
//{
//}

int LightPacket::numLights() const
{
  return (size()-sizeof(PacketHeader))/(sizeof(LightData)+1);
}
int LightPacket::maxLights() const
{
  return s_maxNumberofLights;
}
