#!/usr/bin/python -u 

import serial
import threading
import datetime
import time
from serial.serialutil import SerialException
import DataWriter
import sys
 
g_FailureThreshold = 5

class ArduinoReader(threading.Thread):
    """Parse data from the Arduino"""
    m_Cooling = ""
    m_Heating = ""
    m_Primary = ""
    m_Port = "/dev/ttyS0"
    m_Baudrate = 9600
    m_Timeout = 5
    m_LastUpdate = ""

    #Setup our threading values
    def __init__(self, port, baudrate, timeout):
        threading.Thread.__init__(self)
        self.m_Port = port
        self.m_Baudrate = baudrate
        self.m_Timeout = timeout
        
        

    def run(self):
        self.StartReader(self.m_Port, self.m_Baudrate, self.m_Timeout)

    #Parse out the data from the serial communication channel
    def StartReader(self, port, baudrate, timeout):
        consecutiveFailures = 0;
        while consecutiveFailures < g_FailureThreshold:
            try:
                communicator = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)
                while communicator.isOpen():
                    line = communicator.readline()
                    if line == None or line == "":
                         print "No data received before timeout"
                         consecutiveFailures += 1
                         if(consecutiveFailures > g_FailureThreshold):
                             raise BufferError("Unable to communicate with Serial {0}".format(self.m_Port))
                         continue
                    consecutiveFailures = 0
                    self.UpdateState(line)
            except ValueError as e:
                print "Value error starting the reader{0}".format(e.message)
                consecutiveFailures += 1
                print "Sleeping for " + str(30*consecutiveFailures) + " seconds."
                time.sleep(30 * consecutiveFailures)
            except SerialException as e:
                print "Serial exception trying to open the port{0}".format(e.message)
                consecutiveFailures += 1
                print "Sleeping for " + str(30*consecutiveFailures) + " seconds."
                time.sleep(30 * consecutiveFailures)
            except BufferError as e:
                print "Communication failure{0}".format(e.message)
                consecutiveFailures += 1
                print "Sleeping for " + str(30*consecutiveFailures) + " seconds."
                time.sleep(30 * consecutiveFailures)

    #With our line of input, update our objects within
    def UpdateState(self, line):  
        if line == None or line == "":
            print "No data received before timeout"
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
            time = self.m_LastUpdate
        self.m_LastUpdate = time
        return

    #See if we're valid.  A valid ArduinoReader has non null for heating, 
    #cooling, and temperature
    def IsValid(self):
        retVal = False
        if self.m_Primary != "" and self.m_Heating != "" and self.m_Cooling != "":
            retVal = True
        return retVal

    def ToString(self):        
        retVal = "Time <%s>, Primary <%s>, Heating <%s>, Cooling <%s>" % (datetime.datetime.now(), self.m_Primary, self.m_Heating, self.m_Cooling)
        return retVal

def main():

    while 1:
        print "Starting up connection"
        reader = ArduinoReader("/dev/ttyS1", 9600, 30)
        reader.setDaemon(True)
        reader.start()
        writer = DataWriter.DatabaseWriter()

        while threading.active_count() > 0:
            time.sleep(5)
            if reader.IsValid():
                #Save our data off to the database
                x = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
                writer.writeData([x, reader.m_Primary, reader.m_Heating, reader.m_Cooling])
                print "Added time at " + str(x)
        print "All done"

if  __name__ =='__main__':
    main()





