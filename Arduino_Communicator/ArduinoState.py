from __future__ import with_statement 
import threading

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
        if line == None or line == "":
            print "Invalid line of data received.  Cannot be null."
            return

        list = line.split(":");
        #See if we have valid input
        if len(list) == 1 or len(list) > 2:
            #Invalid, need one colon
            print "Invalid line received: <%s>" % line
            return
        
        #Valid characters
        operator = list[0]
        value = list[1]
        #No need to lock the read/write of our values. Python says this is
        #thread safe        
        if operator == None or value == None:
            print "Invalid line received: <%s>" % line
            return

        #remove trailing whitespace
        value = value.rstrip()
        operator = operator.rstrip()

        #Compare
        time = datetime.datetime.now()

        #As we update our internal state, we need to lock as we
        #could have readers pulling at the same time
        with self.m_Lock:
            if operator.lower() == "internal":            
                self.m_Primary = value                
            elif operator.lower() == "cooling":        
                if value.lower() == "off":
                    self.m_Cooling = 0
                else:
                    self.m_Cooling = 1                   
            elif operator.lower() == "heating":            
                if value.lower() == "off":
                    self.m_Heating = 0
                else:
                    self.m_Heating = 1
            else:
                #Invalid command.  Don't update our time
                time = self.m_LastUpdateTime
            self.m_LastUpdateTime = time

        #Done updating, release the lock with the with statement
        return

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
            retVal = "Time <%s>, Primary <%s>, Heating <%s>, Cooling <%s>" % (self.m_LastUpdateTime, self.m_Primary, self.m_Heating, self.m_Cooling)
        return retVal

    def GetLastUpdatedTime(self):
        return "%s" % (self.m_LastUpdateTime)