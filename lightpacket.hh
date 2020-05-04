#ifndef LIGHTPACKET_HH
#define LIGHTPACKET_HH

#include <QByteArray>
#include "light.hh"

class LightPacket : public QByteArray
{
public:

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
