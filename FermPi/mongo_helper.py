from pymongo import MongoClient
import datetime
import ConfigParser

mongo_enabled = False

config = ConfigParser.ConfigParser()
config.read('settings.cfg')
m_server = config.get('mongo', 'host')
m_port = int(config.get('mongo', 'port'))
try:
    mon_client = MongoClient(m_server, m_port)
    print "MongoDb initialized successfully (%s)" % m_server
    mongo_enabled = True
except Exception as ex:
    print "Caught exception %s while attempting to initialize mongo, will not log!" % ex
    mongo_enabled = False

def storelog(t_carboy, t_chamber, t_room):
    if not mongo_enabled:
        return
    db = mon_client.temperatures
    templog = { "Carboy": t_carboy,
           "Chamber": t_chamber,
           "Room": t_room,
           "date": datetime.datetime.utcnow() }
    post_id = db.fermentation.insert(templog)
    #print(post_id)

def store_heating_on():
    if not mongo_enabled:
        return
    db = mon_client.temperatures
    heat_on_action = { "action" : "ON",
        "date" : datetime.datetime.utcnow() }
    post_id = db.heating.insert(heat_on_action)
    #print(post_id)

def store_cooling_on():
    if not mongo_enabled:
        return
    db = mon_client.temperatures
    cool_on_action = { "action" : "ON",
        "date" : datetime.datetime.utcnow() }
    post_id = db.cooling.insert(cool_on_action)
    #print(post_id)

def store_turnoff():
    if not mongo_enabled:
        return
    db = mon_client.temperatures
    action_off = { "action" : "OFF",
        "date" : datetime.datetime.utcnow() }
    post_id1 = db.heating.insert(action_off)
    post_id2 = db.cooling.insert(action_off)

def get_temp_logs(starting_date = datetime.datetime.utcnow() - datetime.timedelta(days=3), ending_date = datetime.datetime.utcnow() ):
    if not mongo_enabled:
        raise Exception("MongoDb was not initialized, can't retrieve data!")
    # default range is 3 days prior to now
    db = mon_client.temperatures
    return db.fermentation.find({"date": {"$lt": ending_date, "$gt": starting_date}}).sort("date")

# Probably a better way to return this data...
def get_cooling_logs(starting_date = datetime.datetime.utcnow() - datetime.timedelta(days=3), ending_date = datetime.datetime.utcnow() ):
    if not mongo_enabled:
        raise Exception("MongoDb was not initialized, can't retrieve data!")
    # default range is 3 days prior to now
    db = mon_client.temperatures
    return db.cooling.find({"date": {"$lt": ending_date, "$gt": starting_date}}).sort("date")

# Probably a better way to return this data...
def get_heating_logs(starting_date = datetime.datetime.utcnow() - datetime.timedelta(days=3), ending_date = datetime.datetime.utcnow() ):
    if not mongo_enabled:
        raise Exception("MongoDb was not initialized, can't retrieve data!")
    # default range is 3 days prior to now
    db = mon_client.temperatures
    return db.heating.find({"date": {"$lt": ending_date, "$gt": starting_date}}).sort("date")

# Utility functions, not normally used
def get_all_collections():
    db = mon_client.temperatures
    db.collection_names()

def create_index():
    from pymongo import ASCENDING, DESCENDING
    db = mon_client.temperatures
    db.fermentation.create_index([("date", DESCENDING)])
    db.heating.create_index([("date", DESCENDING)])
    db.cooling.create_index([("date", DESCENDING)])
    # removal example
    # db.cooling.remove()
    # db.heating.remove()

# Prints info about existing databases
def diagnostics():
    print mon_client.database_names()
