

import serial
import threading

import time
from serial.serialutil import SerialException
import DataWriter
import sys
from ArduinoState import ArduinoState
g_FailureThreshold = 5

class Communicator(threading.Thread):
    """Parse data from the Arduino"""
   
    m_Port = "/dev/ttyS0"
    m_Baudrate = 9600
    m_Timeout = 5


    #Setup our threading values
    def __init__(self, port, baudrate, timeout):
        threading.Thread.__init__(self)
        self.m_Port = port
        self.m_Baudrate = baudrate
        self.m_Timeout = timeout
        self.m_ArduinoState = ArduinoState()                

    def run(self):
        self.StartReader(self.m_Port, self.m_Baudrate, self.m_Timeout)

    #Parse out the data from the serial communication channel
    def StartReader(self, port, baudrate, timeout):
        consecutiveFailures = 0;
        while consecutiveFailures < g_FailureThreshold:
            #Open a connection to the serial device
            communication = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)
            
            while communication.isOpen():
                #We have a valid connection.  Attempt to read anything we have
                print "hit"
                if communication.inWaiting:
                    #We have data to read
                    line = self.ReadLine(communication)
                    if line == None or False == self.m_ArduinoState.UpdateState(line):
                        #Haven't got a full line yet, or there was an error
                        consecutiveFailures += 1
                        continue
                    else:
                        #Have a valid line, reset our count
                        consecutiveFailures = 0
                #See if we have anything to write to the output stream
                #End of a cycle.  Wait a second
                time.sleep(1)


    def ReadLine(self, input):
        retVal = None
        try:                        
            line = input.readline()
            if line == None or line == "":
                print "No data received before timeout" 
            else:
                retVal = line                
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
        #Return what we found
        return retVal
   

    def ToString(self):        
        retVal = "Time <%s>, Primary <%s>, Heating <%s>, Cooling <%s>" % (self.m_ArduinoState.GetLastUpdatedTime(), 
                                                                          self.m_ArduinoState.m_PrimaryTemperature, 
                                                                          self.m_ArduinoState.m_HeatingState, 
                                                                          self.m_ArduinoState.m_CoolingState)
        return retVal

    def IsValid(self):
        return self.m_ArduinoState.IsValid()







