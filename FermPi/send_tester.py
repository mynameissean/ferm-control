#/usr/bin/python

#
# Written for PyUSB 1.0 (w/libusb 1.0.3)
#


import usb, sys # 1.0 not 0.4
sys.path.append("..")
import time

from digispark.usbdevice import ArduinoUsbDevice

send_bank = ["0\n","1\n","2\n","3\n","5\n","6\n","7\n","8\n"]

if __name__ == "__main__":
   try:
      theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
   except:
      sys.exit("No DigiUSB Device Found")

   while(True):
      for sb in send_bank:
          for c in sb:
              theDevice.write(ord(c))
          time.sleep(5)

   try:
     user_input = sys.argv[1]+"\n"
   except:
     exit("No data to send")
        
   for c in user_input:
     theDevice.write(ord(c))
