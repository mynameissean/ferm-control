import templib as tl
import send
import turnoff
import mongo_helper as mh
import requests

import ConfigParser
config = ConfigParser.ConfigParser()
config.read('settings.cfg')

import time
import datetime

# How far to go past our target temperature when heating/cooling
# Useful for compensating for temperature gauges that aren't placed well
delta_factor = 0.5

# 0 => Always on, don't change this
# 1 == Fridge on, 6=off
# 2 == Fans on, 7=off
# 3 == Heater on, 8=off

class ChamberState:
    idle = 1
    cool = 2
    heat = 3

# TODO: make this optional
base = config.get('external_web_api', 'endpoint')
set_str = base + '?set=%s,%s'

class FermControl():
    # Initial variables
    max_cycles_on = 10 # force fridge to turn off for a cycle
    cur_cycles_on = 0
    cur_mode = "Uninitialized"

    # Temperature settings
    target_temp = 65    # The target temperature
    temp_tolerance = 20 # +/- variance allowed before cooling/heating occur

    my_state = ChamberState.idle

    def min_temp(self):
        return self.target_temp - self.temp_tolerance

    def max_temp(self):
        return self.target_temp + self.temp_tolerance

    def get_temp_settings_external(self):
        self.log_to_file("Attempting to get external settings...")
        try:
            m_set = requests.get(base).text.strip().split(',')
            if len(m_set) != 2:
                print "ERROR! did not find correct temp settings in %s"
            m_target_temp = float(m_set[0])
            m_delta_temp = float(m_set[1])
            self.log_to_file("External settings found successfully, saving %s, %s" % \
              (m_target_temp, m_delta_temp))
            self.target_temp = m_target_temp
            self.temp_tolerance = m_delta_temp
            self.log_to_file("Loading temp settings successful (%s,%s)!" % \
             (m_target_temp, m_delta_temp))
        except Exception as ex:
            self.log_to_file("Error while getting external settings! %s" % ex)

    def set_temp_settings_external(self, m_base, m_delta):
        self.log_to_file("Attempting to save temp settings externally...")
        try:
            res = requests.get(set_str % (m_base, m_delta))
            if res.status_code != 200:
                raise Exception("Setting externally failed with status code %s!" % res.status_cdoe)
        except Exception as ex:
            self.log_to_file("Error while setting external settings! %s" % ex)

    def set_primary_ferment(self):
        self.target_temp = 67
        self.temp_tolerance = 1.5
        self.cur_mode = "Primary fermentation (%s-%s)" % \
          (self.min_temp(), self.max_temp())
        return self.cur_mode

    def set_secondary_ferment(self):
        self.target_temp = 69
        self.temp_tolerance = 2
        self.cur_mode = "Secondary fermentation (%s-%s)" % \
          (self.min_temp(), self.max_temp())
        return self.cur_mode

    def set_custom_temp(self, m_target, m_tolerance):
        self.target_temp = float(m_target)
        self.temp_tolerance = float(m_tolerance)
        self.cur_mode = "Custom range (%s-%s)" % \
          (self.min_temp(), self.max_temp())
        self.set_temp_settings_external(self.target_temp, self.temp_tolerance)
        return self.cur_mode
    
    def set_bottle_carbonating(self):
        self.target_temp = 71
        self.temp_tolerance = 2.5
        self.cur_mode = "Bottle carbonation (%s-%s)" % \
          (self.min_temp(), self.max_temp())
        return self.cur_mode
    
    def set_idle(self):
        self.target_temp = 70
        self.temp_tolerance = 20
        self.turn_off()
        self.cur_mode = "Set temperature control to none/idle!"
        return self.cur_mode
    
    def set_manual(self, cmd_input):
        # Manual test commands
        send.send_to_digi(cmd_input)
        return "Sent %s to digi successfully." % cmd_input

    def log_to_file(self, m_str):
        m_str = str(datetime.datetime.now()) + ": " + m_str
        print(m_str)
        with open("fermentlog.txt", 'a') as myfile:
            myfile.write(m_str + '\n')
    
    #### External queries ####
    
    def get_carboy_temp(self):
        return tl.get_temp('Probe3')
    
    def get_chamber_temp(self):
        return tl.get_temp('Probe2')
    
    def get_room_temp(self):
        return tl.get_temp('Probe1')
    
    def get_temp_range(self):
        return [self.min_temp(), self.max_temp()]
    
    def get_status(self):
        return [self.cur_mode, self.get_carboy_temp()]
    
    ########################

    #### Turn on the various states ####

    def turn_off(self):
        # Turn everything off (fans/cooling/heating)
        send.send_to_digi('6') # fridge off
        send.send_to_digi('7') # fans off
        send.send_to_digi('8') # heating off
        self.cur_cycles_on = 0
        self.my_state = ChamberState.idle
        self.log_to_file("DEBUG: Turned everything off!")

    def cooling_on(self):
        send.send_to_digi('1') # fridge on
        send.send_to_digi('2') # fans on
        send.send_to_digi('8') # heating off
        self.my_state = ChamberState.cool
        self.log_to_file("DEBUG: Turned cooling ON.")

    def heating_on(self):
        send.send_to_digi('6') # fridge off
        send.send_to_digi('2') # fans on
        send.send_to_digi('3') # heating on
        self.my_state = ChamberState.heat
        self.log_to_file("DEBUG: Turning heating ON.")

    ####################################
    
    def control_temp_loop(self):
        # First, set everything to off (known starting state)
        self.turn_off()

        # Load external settings if possible
        self.get_temp_settings_external()

        # Loop forever
        while True:
            try: # top level retry so the thread doesn't die
                t_carboy = tl.get_temp('Probe3')
                t_chamber = tl.get_temp('Probe2')
                t_room = tl.get_temp('Probe1')
                self.log_to_file("Heartbeat - carboy: %s, ambient: %s, room: %s" % (t_carboy, t_chamber, t_room))
                try:  # try to log to db, but this isn't mission critical
                    mh.storelog(t_carboy, t_chamber, t_room)
                except Exception as ex:
                    self.log_to_file("Exception storing temperatures into db! %s" % ex)
                # If we are idle, decide if we need to cool/heat
                if self.my_state == ChamberState.idle:
                    if float(t_carboy) < self.min_temp():
                        self.heating_on()
                    elif float(t_carboy) > self.max_temp():
                        self.cooling_on()
                elif self.my_state == ChamberState.cool:
                    self.cur_cycles_on += 1
                    # Swing slightly past the delta, inefficient temp gauges at the moment
                    if float(t_carboy) < self.target_temp - delta_factor * self.temp_tolerance:
                        self.turn_off()
                    else:
                        self.cooling_on() # shouldn't be set to anything else but just in case...
                elif self.my_state == ChamberState.heat:
                    self.cur_cycles_on += 1
                    # Swing slightly past the delta, inefficient temp gauges at the moment
                    if float(t_carboy) > self.target_temp + delta_factor * self.temp_tolerance:
                        self.turn_off()
                    else:
                        self.heating_on() # shouldn't be set to anything else but just in case...

                # Stop current activity if we've gone [max_cycles_on] without reaching our target
                if self.cur_cycles_on >= self.max_cycles_on:
                    self.turn_off()
                    self.log_to_file("%s: On too long (%s)! Turned system off at %s" %
                      (str(datetime.datetime.now()), self.my_state, t_carboy) )

                time.sleep(10 * 60)
            except LookupError as err:
                self.log_to_file("Temp sensor lookup failed! %s" % err)

if __name__ == "__main__":
    m_class = FermControl()
    m_class.control_temp_loop()
