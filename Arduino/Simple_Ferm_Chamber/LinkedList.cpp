#include "LinkedList.h"

LinkedList::LinkedList()
{
	m_Next = NULL;
}

///<summary> Update the next element in the list to the new item </summary>
///<param name="Next">The new element that is to be addressed next in the list</param>
void LinkedList::SetNext(LinkedList* Next)
{
	m_Next = Next;
}

///<summary> Get the next element in the list </summary>
LinkedList* LinkedList::GetNext()
{
	return m_Next;
}