#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class LinkedList
{
public:
	LinkedList();
	LinkedList* GetNext();
	void SetNext(LinkedList*);
	virtual void Delete();

private:	
	LinkedList* m_Next;
	

};

#endif