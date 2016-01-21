import unittest
from ArduinoState import ArduinoState

class ArduinoStateTest(unittest.TestCase):
    def setUp(self):
        self.m_Arduino = ArduinoState()

class InputTests(ArduinoStateTest):

    def test_NullInput(self):        
        self.assertFalse(self.m_Arduino.UpdateState(""), "Empty string testing")
        self.assertFalse(self.m_Arduino.UpdateState(None), "None Testing")
    
    def test_InvalidLineStructure(self):
        self.assertFalse(self.m_Arduino.UpdateState("asdf"), "Too few :")
        self.assertFalse(self.m_Arduino.UpdateState("asdf:asdf:asdf"), "Too many :")
        self.assertFalse(self.m_Arduino.UpdateState(":"), "No Operator or Value")
        self.assertFalse(self.m_Arduino.UpdateState(":On"), "No Operator")
        self.assertFalse(self.m_Arduino.UpdateState("Cooling:"), "No Value")

    def test_NoChangeInInvalids(self):
        #Set our initial state
        self.m_Arduino.UpdateState("Cooling:On")
        self.m_Arduino.UpdateState("internal:76")

        #Now check that everything got set ok
        self.assertEqual(self.m_Arduino.m_CoolingState, 1, "Cooling set to invalid state")
        self.assertEqual(self.m_Arduino.m_PrimaryTemperature, "76", "Temperature should be 76, not <%s>" % (self.m_Arduino.m_PrimaryTemperature))

        #Run some invalids
        self.test_InvalidLineStructure()

        #Make sure nothing got changed again
        self.assertEqual(self.m_Arduino.m_CoolingState, 1, "Cooling set to invalid state")
        self.assertEqual(self.m_Arduino.m_PrimaryTemperature, "76", "Temperature should be 76, not <%s>" % (self.m_Arduino.m_PrimaryTemperature))
        

if __name__ == '__main__':
    try:
        unittest.main()
    except SystemExit as inst:
        if inst.args[0] is True: # raised by sys.exit(True) when tests failed
            #raise
            pass