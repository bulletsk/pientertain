#ifndef LIGHTPACKET_HH
#define LIGHTPACKET_HH

#include <QByteArray>

class LightPacket : public QByteArray
{
public:

  struct PacketHeader {
    uint8_t protocol[9];
    uint8_t versionMajor;
    uint8_t versionMinor;
    uint8_t sequenceNumber;
    uint16_t reserved1;
    uint8_t colorSpace;
    uint8_t reserved2;
  };

  struct LightData {
    uint16_t id;
    uint16_t r;
    uint16_t g;
    uint16_t b;
  };

  LightPacket();

  void setSequenceNumber(int num);

//  void addLightData(unsigned short id, unsigned short x, unsigned short y, unsigned short brightness);
  void addLightData(const uint16_t id, const uint16_t r, const uint16_t g, const uint16_t b);

  int numLights() const;
  int maxLights() const;
};

#endif // LIGHTPACKET_HH
