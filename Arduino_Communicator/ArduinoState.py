from __future__ import with_statement 
from threading import Lock
import datetime

class ArduinoState():
    m_CoolingState = ""
    m_HeatingState = ""
    m_PrimaryTemperature = ""
    m_LastUpdateTime = ""

    def __init__(self):
        #Create a lock object
        self.m_Lock = Lock()

    #With our line of input, update our objects within
    def UpdateState(self, line):  
        bRetVal = False
        if line == None or line == "":
            print "Invalid line of data received.  Cannot be null."
            return bRetVal

        list = line.split(":");
        #See if we have valid input
        if len(list) == 1 or len(list) > 2:
            #Invalid, need one colon
            print "Invalid line received: <%s>" % line
            return bRetVal
        
        #Valid characters
        operator = list[0]
        value = list[1]
        #No need to lock the read/write of our values. Python says this is
        #thread safe        
        if operator == None or operator == "" or value == None or value == "":
            print "Invalid line received: <%s>" % line
            return bRetVal

        
        #remove trailing whitespace
        value = value.rstrip()
        operator = operator.rstrip()

        #Compare
        time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')

        #As we update our internal state, we need to lock as we
        #could have readers pulling at the same time
        with self.m_Lock:
            if operator.lower() == "internal":            
                self.m_PrimaryTemperature = value       
                bRetVal = True         
            elif operator.lower() == "cooling":        
                if value.lower() == "off":
                    self.m_CoolingState = 0
                    bRetVal = True
                else:
                    self.m_CoolingState = 1                   
                    bRetVal = True
            elif operator.lower() == "heating":            
                if value.lower() == "off":
                    self.m_HeatingState = 0
                    bRetVal = True
                else:
                    self.m_HeatingState = 1
                    bRetVal = True
            else:
                #Invalid command.  Don't update our time
                print "Invalid command received <%s>" % operator
                time = self.m_LastUpdateTime
            self.m_LastUpdateTime = time

        #Done updating, release the lock with the with statement
        return bRetVal

    #See if we're valid.  A valid ArduinoReader has non null for heating, 
    #cooling, and temperature
    def IsValid(self):
        retVal = False
        if self.m_LastUpdateTime != "" and self.m_PrimaryTemperature != "" and self.m_HeatingState != "" and self.m_CoolingState != "":
            retVal = True
        return retVal
    
    def ToString(self):     
        #Lock the read state of the variables
        retVal = None
        with self.m_Lock:
            retVal = "Time <%s>, Primary <%s>, Heating <%s>, Cooling <%s>" % (self.m_LastUpdateTime, self.m_PrimaryTemperature, self.m_HeatingState, self.m_CoolingState)
        return retVal

    def GetLastUpdatedTime(self):
        return "%s" % (self.m_LastUpdateTime)