#ifndef LIGHTPACKET_HH
#define LIGHTPACKET_HH

#include <QByteArray>

class LightPacket : public QByteArray
{
public:
  LightPacket();

  void setSequenceNumber(int num);

//  void addLightData(unsigned short id, unsigned short x, unsigned short y, unsigned short brightness);
  void addLightData(unsigned short id, unsigned short r, unsigned short g, unsigned short b);

  int numLights() const;
  int maxLights() const;
};

#endif // LIGHTPACKET_HH
