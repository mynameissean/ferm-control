#/usr/bin/python

#
# Written for PyUSB 1.0 (w/libusb 1.0.3)
#


import usb, sys # 1.0 not 0.4
sys.path.append("..")

from digispark.usbdevice import ArduinoUsbDevice

# Caller handles exceptions
def send_to_digi(m_str):
    m_str += '\n'
    try:
       theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
    except:
       #sys.exit("No DigiUSB Device Found")
       return "ERROR: No DigiUSB Device Found"
    for c in m_str:
        theDevice.write(ord(c))

if __name__ == "__main__":
    send_to_digi(sys.argv[1])
    try:
        user_input = sys.argv[1]
        send_to_pi(user_input)
    except:
        exit("No data provided on cmd line to send!")

