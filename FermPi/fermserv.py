#!/usr/bin/env python
'''
Serve up some fermentation related pages
'''

# Web server section
import gevent
from gevent import monkey
monkey.patch_all() # This must be done as soon as possible, otherwise breaks subprocess!
# https://github.com/surfly/gevent/issues/446

import ConfigParser
config = ConfigParser.ConfigParser()
config.read('settings.cfg')

import mongo_helper as mh
import ferment as fer

# SFTP credentials (to store graph remotely) storage
# TODO make this optional
import paramiko
import sys
m_username = config.get('external_sftp', 'username')
m_password = config.get('external_sftp', 'password')
m_host = config.get('external_sftp', 'host')

from gevent.wsgi import WSGIServer
import json
from flask import Flask, jsonify, Response

# Matplotlib section
import time
import datetime
import cStringIO
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# Instance of ferm chamber control
m_fer = fer.FermControl()

APP = Flask(__name__)

def store_temp_graph_remote(data):
    # this should never raise an exception or the thread wil ldie
    try:
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(m_host, username=m_username, password=m_password)
        sftp = client.open_sftp()
        rfile = sftp.open('ferm/graph_day.png', 'w')
        rfile.write(data)
        rfile.close()
    except Exception as ex:
        print("Caught exception while storing graph remotely: %s" % ex)

def get_temp_history(from_datetime, to_datetime = None, store_remote = False):
    if to_datetime == None:
        to_datetime = datetime.datetime.utcnow()
    settings = m_fer.get_temp_range()
    dates  = []
    carboy  = []
    secondary = []
    room = []
    mintemp = []
    maxtemp = []
    for curt in mh.get_temp_logs(starting_date = from_datetime, ending_date = to_datetime):
        dates.append(curt['date'])
        carboy.append(curt['Carboy'])
        secondary.append(curt['Chamber'])
        room.append(curt['Room'])
        mintemp.append(settings[0])
        maxtemp.append(settings[1])
    m_dates = matplotlib.dates.date2num(dates)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    
    ax.plot_date(m_dates, mintemp, 'g--')
    ax.plot_date(m_dates, maxtemp, 'g--')
    ax.grid()
    ax.fill_between(m_dates, mintemp, maxtemp, facecolor='green', alpha=0.2)
    ax.plot_date(m_dates, carboy, 'b-', linewidth=2.5)
    ax.plot_date(m_dates, secondary, 'k-', linewidth=1)
    ax.plot_date(m_dates, room, 'r-', linewidth=1.5)
    ax.autoscale_view()

    ### Plot cooling and heating vertical section on the graph section ###
    cur_on = False
    on_time = None
    for cool in mh.get_cooling_logs(starting_date = from_datetime, ending_date = to_datetime):
        if not cur_on and cool['action'] == "ON":
            cur_on = True
            on_time = cool['date']
        elif cur_on and cool['action'] == "OFF":
            cur_on = False
            ax.axvspan(on_time, cool['date'], facecolor="blue", alpha=0.3)
    if cur_on == True:
        ax.axvspan(on_time, to_datetime, facecolor="blue", alpha=0.3)


    cur_on = False
    on_time = None
    for heat in mh.get_heating_logs(starting_date = from_datetime, ending_date = to_datetime):
        if not cur_on and heat['action'] == "ON":
            cur_on = True
            on_time = heat['date']
        elif cur_on and heat['action'] == "OFF":
            cur_on = False
            ax.axvspan(on_time, heat['date'], facecolor="red", alpha=0.3)
    if cur_on == True:
        ax.axvspan(on_time, to_datetime, facecolor="red", alpha=0.3)
    #####################################################

    fig.autofmt_xdate()
    sio = cStringIO.StringIO()
    fig.savefig(sio, format='png')
    # delete figure to prevent memory leaks!
    fig.clf()
    plt.close()
    if store_remote == True:
        store_temp_graph_remote(sio.getvalue())
    return Response(sio.getvalue(), mimetype='image/png')

@APP.route("/")
def home():
    return (str(m_fer.get_status()) + "\n\n" + str(m_fer.get_temp_range()) + "\n\n" + "Options: /ferment /ferment2 /set_temp/<min_val>/<max_val> /mash/<temp> /manualcmd/# /turnoff /status /temp_day.png /temp_week.png /graph/<custom_days_from> /graph2/<custom_days_from>/<custom_days_to>, /probe/primary, /probe/secondary")

@APP.route("/ferment")
def ferment_now():
    return m_fer.set_primary_ferment()

@APP.route("/ferment2")
def ferment_secondary_now():
    return m_fer.set_secondary_ferment()

@APP.route("/set_temp/<min_val>/<max_val>")
def set_temp_custom(min_val, max_val):
    return m_fer.set_custom_temp(min_val, max_val)

@APP.route("/mash/<target_temp>")
def set_mash_temp(target_temp):
    return m_fer.set_mash(target_temp)

@APP.route("/manualcmd/<inval>")
def set_manual(inval):
    # This sends specified command to arduino, for manual & debug only
    return m_fer.set_manual(inval[:1])

@APP.route("/probe/primary")
def set_primary():
    # Set the primary probe as the temperature target
    return "Set selected probe to %s" % str(m_fer.set_probe_primary())

@APP.route("/probe/secondary")
def set_secondary():
    # Set the secondary probe as the temperature target
    return "Set selected probe to %s" % str(m_fer.set_probe_secondary())

@APP.route("/fans/alwayson")
def fans_alwayson():
    # Override fans to on
    return m_fer.set_fans_always_on(True)

@APP.route("/fans/default")
def fans_default():
    # Fans have default behavior
    return m_fer.set_fans_always_on(False)

@APP.route("/turnoff")
def turn_off_now():
    return m_fer.set_idle()

@APP.route("/status")
def get_status():
    # self refreshing status page
    status = m_fer.get_status()
    if status[1] == 1:
        cur_probe = "Primary"
    elif status[1] == 2:
        cur_probe = "Secondary"
    else:
        cur_probe = "Unknown"
    html = """<html>
      <head><meta http-equiv="refresh" content="300" ><title>FermStatus</title>
      <script type="text/javascript">
        function primary() {
          var xmlHttp = new XMLHttpRequest();
          xmlHttp.open( "GET", "/ferment", false );
          xmlHttp.send( null );
          return false;
        }
        function secondary() {
          var xmlHttp = new XMLHttpRequest();
          xmlHttp.open( "GET", "/ferment2", false );
          xmlHttp.send( null );
          return false;
        }
        function turnoff() {
          var xmlHttp = new XMLHttpRequest();
          xmlHttp.open( "GET", "/turnoff", false );
          xmlHttp.send( null );
          return false;
        }
        function probe_primary() {
          var xmlHttp = new XMLHttpRequest();
          xmlHttp.open( "GET", "/probe/primary", false );
          xmlHttp.send( null );
          return false;
        }
        function probe_secondary() {
          var xmlHttp = new XMLHttpRequest();
          xmlHttp.open( "GET", "/probe/secondary", false );
          xmlHttp.send( null );
          return false;
        }
        function fans_override_on() {
          var xmlHttp = new XMLHttpRequest();
          xmlHttp.open( "GET", "/fans/alwayson", false );
          xmlHttp.send( null );
          return false;
        }
        function fans_override_off() {
          var xmlHttp = new XMLHttpRequest();
          xmlHttp.open( "GET", "/fans/default", false );
          xmlHttp.send( null );
          return false;
        }
      </script>
      </head>
      <body>
      <img src="/temp_day.png"></img>
      <br>
      <p style="font-size:20px">
      Status: %s, Probe: %s
      <br>
      Primary Carboy: %s, Secondary Carboy: %s, Room: %s
      <br>
      <form action="/status" method="get">
      <button type="button" onclick="primary()" style="font-size:20px">Primary Ferment (66-68)</button>
      <button type="button" onclick="secondary()" style="font-size:20px">Secondary/Conditioning (67-71)</button>
      <button type="button" onclick="turnoff()" style="font-size:20px">Turn OFF</button>
      <br><br>
      <button type="button" onclick="probe_primary()" style="font-size:20px">Probe Primary</button>
      <button type="button" onclick="probe_secondary()" style="font-size:20px">Probe Secondary</button>
      <br><br>
      <button type="button" onclick="fans_override_on()" style="font-size:20px">Fan Override - Always On</button>
      <button type="button" onclick="fans_override_off()" style="font-size:20px">Default Fan Behavior</button>
      </form>
      </p>
      </body>
      </html>
      """
    return html % (str(status[0]), cur_probe, m_fer.get_carboy_temp(), m_fer.get_secondary_temp(), m_fer.get_room_temp())

@APP.route("/temp_day.png")
def get_temp_day():
    return get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=1))

@APP.route("/temp_week.png")
def get_temp_week():
    return get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=7))

@APP.route("/graph/<custom_days_from>")
def get_temp_custom(custom_days_from):
    return get_temp_custom_detailed(custom_days_from, 0)

@APP.route("/graph2/<custom_days_from>/<custom_days_to>")
def get_temp_custom_detailed(custom_days_from, custom_days_to):
    return get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=int(custom_days_from)), datetime.datetime.utcnow() - datetime.timedelta(days=int(custom_days_to)))

def run_ferment_thread():
    # This will run forever until process is killed
    m_fer.control_temp_loop()

def upload_graph():
    gevent.spawn_later(60*60, upload_graph) # run again in an hour)
    # Bug: graph can be written before settings are loaded async on the first run,
    #      sleep to compensate
    time.sleep(30)
    # Update remote graph every hour
    print "Heartbeat at time %s, updating remote graph" % datetime.datetime.now()
    get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=3), store_remote = True)

if __name__ == "__main__":
    gevent.spawn(run_ferment_thread)
    gevent.spawn(upload_graph)
    APP.debug = True
    server = WSGIServer(('', 8025), APP)
    server.serve_forever()
