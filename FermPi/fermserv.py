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

# SFTP credentials (to store graph remotely)storage
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
import datetime
import cStringIO
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# Instance of ferm chamber control
m_fer = fer.FermControl()

APP = Flask(__name__)

def store_temp_graph_remote(data):
    # this should never raise an exception!
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

def get_temp_history(from_datetime, store_remote = False):
    settings = m_fer.get_temp_range()
    dates  = []
    carboy  = []
    chamber = []
    room = []
    mintemp = []
    maxtemp = []
    for curt in mh.get_temp_logs(starting_date = from_datetime):
        dates.append(curt['date'])
        carboy.append(curt['Carboy'])
        chamber.append(curt['Chamber'])
        room.append(curt['Room'])
        mintemp.append(settings[0])
        maxtemp.append(settings[1])
    m_dates = matplotlib.dates.date2num(dates)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.plot_date(m_dates, carboy, 'b-')
    ax.plot_date(m_dates, chamber, 'r-')
    ax.plot_date(m_dates, room, 'g-')
    ax.plot_date(m_dates, mintemp, 'y--')
    ax.plot_date(m_dates, maxtemp, 'r--')
    ax.autoscale_view()
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
    return (str(m_fer.get_status()) + "\n\n" + str(m_fer.get_temp_range()) + "\n\n" + "Options: /ferment /ferment2 /set_temp/<min_val>/<max_val> /mash/<temp> /manualcmd/# /turnoff /status /temp_day.png /temp_week.png /graph/<custom_days>")

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

@APP.route("/turnoff")
def turn_off_now():
    return m_fer.set_idle()

@APP.route("/status")
def get_status():
    # self refreshing status page
    html = """<html>
      <head><meta http-equiv="refresh" content="300" ><title>FermStatus</title></head>
      <body>
      <img src="/temp_day.png"></img>
      <br>
      Carboy: %s, Ambient: %s, Room: %s
      </body>
      </html>
      """
    return html % (m_fer.get_carboy_temp(), m_fer.get_chamber_temp(), m_fer.get_room_temp())

@APP.route("/temp_day.png")
def get_temp_day():
    return get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=1))

@APP.route("/temp_week.png")
def get_temp_week():
    return get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=7))

@APP.route("/graph/<custom_days>")
def get_temp_custom(custom_days):
    return get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=int(custom_days)))

def run_ferment_thread():
    # This will run forever until process is killed
    m_fer.control_temp_loop()

def t():
    # Update remote graph every hour
    print "Heartbeat at time %s, updating remote graph" % datetime.datetime.now()
    get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=3), store_remote = True)
    gevent.spawn_later(60*60, t)

if __name__ == "__main__":
    gevent.spawn(run_ferment_thread)
    gevent.spawn(t)
    APP.debug = True
    server = WSGIServer(('', 8025), APP)
    server.serve_forever()
