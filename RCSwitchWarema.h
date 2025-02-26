#pragma once

#include <RCSwitch.h>

class RCSwitchWarema : public RCSwitch
{
  using Base = RCSwitch;
  
public:
    void sendMC(const char* code, int dLen, int sLen, int repeat, int delayUs);
    void enableTransmit(int nTransmitterPin);

private:
    int m_transmitterPin{ -1 };
};

void RCSwitchWarema::enableTransmit(int nTransmitterPin)
{
    m_transmitterPin = nTransmitterPin;
    Base::enableTransmit(nTransmitterPin);
}

void RCSwitchWarema::sendMC(const char* sCodeWord, 
                            int dataLength, 
                            int syncLength, 
                            int sendCommand, 
                            int sendDelay )
{
    // For each repeat
    for (int nRepeat = 0; nRepeat < sendCommand; nRepeat++)
    {
        // Start low
        digitalWrite(m_transmitterPin, LOW);  

        // Transmit each character
        int len = strlen(sCodeWord);
        for (int i = 0; i < len; i++) {
            switch (sCodeWord[i]) {
                case 's':
                    digitalWrite(m_transmitterPin, LOW);
                    delayMicroseconds(syncLength);
                    break;

                case 'S':
                    digitalWrite(m_transmitterPin, HIGH);
                    delayMicroseconds(syncLength);
                    break;

                case '0':
                    digitalWrite(m_transmitterPin, HIGH);
                    delayMicroseconds(dataLength / 2);
                    digitalWrite(m_transmitterPin, LOW);
                    delayMicroseconds(dataLength / 2);
                    break;

                case '1':
                    digitalWrite(m_transmitterPin, LOW);
                    delayMicroseconds(dataLength / 2);
                    digitalWrite(m_transmitterPin, HIGH);
                    delayMicroseconds(dataLength / 2);
                    break;
            }
        }
        // End low
        digitalWrite(m_transmitterPin, LOW);   

        // If not the last repeat, wait the specified delay
        if (nRepeat != sendCommand - 1) {
            delayMicroseconds(sendDelay);
        }
    }
}