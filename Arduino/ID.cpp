#include "ID.h"

ID::ID(char* FriendlyName, int FriendlyNameLength, int Index)
{
    m_FriendlyName = FriendlyName;
    m_FriendlyNameLength = FriendlyNameLength;
    m_Index = Index;
}