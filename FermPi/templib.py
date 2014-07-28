'''
templib.py
Temperature sensor library, based on:
https://learn.adafruit.com/adafruits-raspberry-pi-lesson-11-ds18b20-temperature-sensing/software
'''

import os
import glob
import time
import subprocess
 
os.system('modprobe w1-gpio')
os.system('modprobe w1-therm')
 
mapping = []

named = {'28-0000056b2059' : 'Probe1',
'28-0000056b73ae' : 'Probe2',
'28-0000056b416c' : 'Probe3',
}

def lookup_friendly_name(probe_filename):
    for nam in named:
        if nam in probe_filename:
            return named[nam]
    return probe_filename

base_dir = '/sys/bus/w1/devices/'
device_folders = glob.glob(base_dir + '28*')
for devf in device_folders:
    mapping += [str(devf) + '/w1_slave']
    print('Found temp sensor: ' + str(devf))
 
# "safe" - subprocess the file read
def read_temp_raw_safe(sensor):
	catdata = subprocess.Popen(['cat',sensor], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	out,err = catdata.communicate()
	out_decode = out.decode('utf-8')
	lines = out_decode.split('\n')
	return lines
 
def read_temp(sensor):
    lines = read_temp_raw_safe(sensor)
    while lines[0].strip()[-3:] != 'YES':
        time.sleep(0.2)
        lines = read_temp_raw_safe(sensor)
    equals_pos = lines[1].find('t=')
    if equals_pos != -1:
        temp_string = lines[1][equals_pos+2:]
        temp_c = float(temp_string) / 1000.0
        temp_f = temp_c * 9.0 / 5.0 + 32.0
        #return temp_c, temp_f
        return temp_f

def get_temp(m_tname):
    for m_map in mapping:
        for nam in named:
            if nam in m_map and m_tname in named[nam]:
                 return read_temp(m_map)
    return "NONE!  Didn't find temp sensor %s" % m_tname

if __name__ == "__main__":
    file_log_freq = 5
    counter = 0
    # Just log temp to a file
    while True:
        counter += 1
        temp = get_temp('Probe3')
        if counter % file_log_freq == 0:
            with open("test.txt", "a") as myfile:
                myfile.write("%s,%s\n" % ( int(time.time()), temp) )
        print("%s - %s" % ('Probe3', temp) )
        time.sleep(60)
