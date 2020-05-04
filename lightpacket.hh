#ifndef LIGHTPACKET_HH
#define LIGHTPACKET_HH

#include <QByteArray>
#include "light.hh"

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

  LightPacket(bool useRGB, bool useGammaCompensation);

  void setSequenceNumber(int num);

  void addLightData(const Light &light);

  int numLights() const;
  int maxLights() const;

private:
  bool m_sendRGB;
  bool m_useGammaCompensation;
};

#endif // LIGHTPACKET_HH
