#!/usr/bin/python -u 

import time
from Communicator import Communicator
import DataWriter
import threading
import datetime

def main():

    while 1:
        print "Starting up connection"
        reader = Communicator("/dev/ttyS1", 9600, 30)
        reader.setDaemon(True)
        reader.start()
        writer = DataWriter.DatabaseWriter()

        while threading.active_count() > 0:
            time.sleep(5)                        
            if reader.IsValid():
                #Save our data off to the database if we've had an update
                #TODO: Change this so that instead of just taking this time as our value,
                #we only update the updated time on the ArduinoState whenever the primary temperature got a new value.  
                #Then we can use that value to update our database
                x = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S.%f')
                writer.writeData([x, 
                                    reader.m_ArduinoState.m_PrimaryTemperature, 
                                    reader.m_ArduinoState.m_HeatingState, 
                                    reader.m_ArduinoState.m_CoolingState])
                print reader.ToString() 
        print "All done"

if  __name__ =='__main__':
    main()