#!/usr/bin/env python
'''
Serve up some fermentation related pages
'''

import mongo_helper as mh

# Web server section
import gevent
from gevent import monkey
monkey.patch_all()

from gevent.wsgi import WSGIServer
import json
from flask import Flask, jsonify, Response

# Matplotlib section
import datetime
import cStringIO
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

APP = Flask(__name__)

def get_temp_history(from_datetime):
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
        mintemp.append(67)
        maxtemp.append(70)
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
    return Response(sio.getvalue(), mimetype='image/png')

@APP.route("/")
def home():
    return "It's alive!"

@APP.route("/example.png")
def get_temp_last_week():
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.plot([1,2,3])
    #fig.savefig('test.png') # saves to a file on disk
    sio = cStringIO.StringIO()
    fig.savefig(sio, format='png')
    return Response(sio.getvalue(), mimetype='image/png')

@APP.route("/temp_week.png")
def get_temp_week():
    return get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=7))

@APP.route("/temp_day.png")
def get_temp_day():
    return get_temp_history(datetime.datetime.utcnow() - datetime.timedelta(days=1))

def t():
    # Placeholder, this will hold the ferm thread initialization
    print "Heartbeat..."
    gevent.spawn_later(60*60*24, t)

if __name__ == "__main__":
    gevent.spawn(t)
    APP.debug = True
    server = WSGIServer(('', 8025), APP)
    server.serve_forever()
