#include "Definitions.h"
#include "Utility.h"
#include "I2CDisplay.h"
#include <Wire.h>



I2CDisplay::I2CDisplay(int Address)
{
    m_Address = Address;
    m_CurrentPosition = 0;

    //Open up communication with this address
    Wire.beginTransmission(Address);

    //See if we're valid
    if(0 == Wire.endTransmission())
    {
        //Valid return from ending transmission
        Wire.beginTransmission(Address);
        m_IsValid = true;
    }
    else {
        m_IsValid = false;
    }
}

///<summary> Close the connection to the open I2C device</summary>
void I2CDisplay::Disconnect()
{
    Wire.endTransmission();
}

