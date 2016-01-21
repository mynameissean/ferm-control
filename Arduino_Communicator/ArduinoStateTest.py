import unittest
import ArduinoState

class ArduinoStateTest(unittest.TestCase):
    def setUp(self):
        self.m_Arduino = ArduinoState()

class InputTests(ArduinoStateTest):

    def Test(self):        
        self.assertIsNone(self.m_Arduino.UpdateState(""), "Null testing")


if __name__ == '__main__':
    unittest.main()