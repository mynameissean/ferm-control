import csv
import MySQLdb
from decimal import Decimal

class DataWriter(object):
    """Write out the data to our preferred logging mechanism"""

    #Write out the data stream to our preferred location
    def __init__(self):
        #Do Nothing  
        pass      

    def __del__(self):
        #Do nothing
        pass
    
    def writeData(self, iteratorable):        
        #Must be defined by base class
        raise NotImplementedError("Please Implement this method")
        

class DatabaseWriter(DataWriter):
    def __init__(self):
        #For now, do nothing
        DataWriter.__init__(self)
        self.db = MySQLdb.connect(host="localhost", # your host, usually localhost
                             user="LeannBocsa", # your username
                             passwd="unbr34k4b73", # your password
                             db="LeannBocsa") # name of the data base

        # you must create a Cursor object. It will let
        #  you execute all the queries you need
        self.cur = self.db.cursor() 

    def __del__(self):
        DataWriter.__del__(self)
        self.cur.close();
        self.db.close();

    def writeData(self, iteratorable):        
        self.cur.execute('''INSERT into beerviewer_readings (timestamp, primary_temp, heater_state, cooling_state)
                  values (%s, %s, %s, %s)''',
                  (iteratorable[0], iteratorable[1], iteratorable[2], iteratorable[3]))
        self.db.commit()
        pass

class CSVWriter(DataWriter):
    #Write out the data stream to our preferred location
    def __init__(self):
        DataWriter.__init__(self)
        #For now, we'll be using CSV files
        #Prepare our stream writer
        self.m_CSVFile = open("/home/pi/data/readings.csv", "wb");
        self.m_CSVWriter = csv.writer(self.m_CSVFile);
        

    def __del__(self):
        DataWriter.__del__(self)
        self.m_CSVFile.close();
        

    #Write out a line of data to our storage mechanism
    def writeData(self, iteratorable):        
        self.m_CSVWriter.writerow(iteratorable);
        self.m_CSVFile.flush()
        

