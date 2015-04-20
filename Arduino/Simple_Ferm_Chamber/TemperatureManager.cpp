#include "TemperatureManager.h"
#include "Definitions.h"

TemperatureManager::TemperatureManager(Relay *Heating, Relay *Cooling, TemperatureSensor *Primary)
{
    m_Heating = Heating;
    m_Cooling = Cooling;
    m_PrimaryTemp = Primary;
}