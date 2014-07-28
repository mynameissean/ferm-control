import templib as tl
import send
import turnoff
import mongo_helper as mh

import time
import datetime

# 0 not working!
# 1 == Fridge on, 6=off
# 2 == Fans on, 7=off
# 3 == Fridge cooling fan on, 8=off

# Initial variables
max_cycles_on = 10 # force fridge to turn off for a cycle
cur_fr_cycles = 0
beer_name = "NoName Beer"
cur_mode = "Standby"
is_on = False

def set_primary_ferment():
    global min_temp
    global max_temp
    min_temp = 63
    max_temp = 65

def set_secondary_ferment():
    global min_temp
    global max_temp
    min_temp = 67
    max_temp = 71

def set_carbonating():
    global min_temp
    global max_temp
    min_temp = 68
    max_temp = 72

def set_idle():
    global min_temp
    global max_temp
    min_temp = 0
    max_temp = 130

# Starting configuration for me now is "secondary"
set_carbonating()

def log_to_file(m_str):
    m_str = str(datetime.datetime.now()) + ": " + m_str
    print(m_str)
    with open("fermentlog.txt", 'a') as myfile:
        myfile.write(m_str + '\n')

def startup():
    # Turn everything off except fans
    # turnoff.doit() leaving off for now due to debugging
    global is_on
    global min_temp
    global max_temp
    send.send_to_digi('2')
    send.send_to_digi('3')
    is_on = False
    log_to_file("Startup, min temp %s, max temp %s" %
      (min_temp, max_temp) )

startup()

#### External query ####

def get_carboy_temp():
    return t1.get_temp('Probe3')

def get_temp_range():
    global min_temp
    global max_temp
    return [min_temp, max_temp]

def get_status():
    # ["Beer Name", "Mode", "Is_On"]
    global beer_name, cur_mode, is_on
    return [beer_name, cur_mode, is_on]

########################

def control_temp_loop():
    global is_on
    global min_temp
    global max_temp
    global cur_fr_cycles
    while True:
        t_carboy = tl.get_temp('Probe3')
        t_chamber = tl.get_temp('Probe2')
        t_room = tl.get_temp('Probe1')
        log_to_file("Heartbeat - carboy: %s, ambient: %s, room: %s" % (t_carboy, t_chamber, t_room))
        try:  # try to log to db, but this isn't mission critical
            mh.storelog(t_carboy, t_chamber, t_room)
        except Exception as ex:
            print("Exception storing into db! %s" % ex)
        if float(t_carboy) > max_temp:
            cur_fr_cycles += 1
            send.send_to_digi('1') # Cooling on
            send.send_to_digi('2') # Fans on
            if not is_on:
                log_to_file("%s: Cooling ON at %s" %
                  (str(datetime.datetime.now()), t_carboy) )
                is_on = True
        elif float(t_carboy) < min_temp:
            send.send_to_digi('6') # cooling off
            send.send_to_digi('7') # fans off, results in a slower warm-up
            cur_fr_cycles = 0
            if is_on:
                log_to_file("%s: Cooling OFF at %s" %
                  (str(datetime.datetime.now()), t_carboy) )
                is_on = False

        # Give the cooling a break for 1 cycle if we've been on for awhile
        # Whatever reason we've run this long clearly isn't working
        if cur_fr_cycles >= max_cycles_on:
            # should only trigger if we've been on for 3 consecutive cycles
            send.send_to_digi('6')
            cur_fr_cycles = 0
            if is_on:
                log_to_file("%s: On too long! Turned fridge off at %s" %
                  (str(datetime.datetime.now()), t_carboy) )
                is_on = False

        time.sleep(10 * 60)

if __name__ == "__main__":
    control_temp_loop()
