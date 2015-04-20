#include "ID.h"

///<summary>Create a new identifier with the requisite fields</summary>
///<param name="FriendlyName">The name of the identifier</param>
///<param name="FriendlyNameLength">How long the friendly name is</param>
///<param name="Index">Indexed identifier</param>
ID::ID(char* FriendlyName, int FriendlyNameLength, int Index)
{
    m_FriendlyName = FriendlyName;
    m_FriendlyNameLength = FriendlyNameLength;
    m_Index = Index;
}

///<summary>Create a new identifier with the requisite fields</summary>
///<param name="FriendlyName">The name of the identifier</param>
///<param name="FriendlyNameLength">How long the friendly name is</param>
///<param name="Index">Indexed identifier</param>
///<param name="EEPROMAddress">The address in EEPROM where values will be stored</param>
ID::ID(char* FriendlyName, int FriendlyNameLength, int Index, int EEPROMAddress)
{
    m_FriendlyName = FriendlyName;
    m_FriendlyNameLength = FriendlyNameLength;
    m_Index = Index;
    m_EEPROMAddress = EEPROMAddress;
}