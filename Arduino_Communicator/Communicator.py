

import serial
import threading

import time
from serial.serialutil import SerialException
import DataWriter
import sys
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

    def run(self):
        self.StartReader(self.m_Port, self.m_Baudrate, self.m_Timeout)

    #Parse out the data from the serial communication channel
    def StartReader(self, port, baudrate, timeout):
        consecutiveFailures = 0;
        while consecutiveFailures < g_FailureThreshold:
            #Open a connection to the serial device
            input = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)
            while input.isOpen():
                #We have a valid connection.  Attempt to read anything we have
                if input.inWaiting:
                    #We have data to read
                    line = ReadLine(input)
                    if line == None or False == self.UpdateState(line):
                        #Haven't got a full line yet, or there was an error
                        consecutiveFailures += 1
                        continue
                    else:
                        #Have a valid line, reset our count
                        consecutiveFailures = 0


    def ReadLine(self, input):
        retVal = None
        try:                        
            line = input.readline()
            if line == None or line == "":
                print "No data received before timeout"                                           
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

   

    def ToString(self):        
        retVal = "Time <%s>, Primary <%s>, Heating <%s>, Cooling <%s>" % (datetime.datetime.utcnow(), self.m_Primary, self.m_Heating, self.m_Cooling)
        return retVal







