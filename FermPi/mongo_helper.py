from pymongo import MongoClient
import datetime
import ConfigParser

# TODO make mongo optional
config = ConfigParser.ConfigParser()
config.read('settings.cfg')
m_server = config.get('mongo', 'host')
m_port = int(config.get('mongo', 'port'))
mon_client = MongoClient(m_server, m_port)

def storelog(t_carboy, t_chamber, t_room):
    db = mon_client.temperatures
    import datetime
    templog = { "Carboy": t_carboy,
           "Chamber": t_chamber,
           "Room": t_room,
           "date": datetime.datetime.utcnow() }
    post_id = db.fermentation.insert(templog)
    #print(post_id)

# Utility functions, not normally used
def get_all_collections():
    db = mon_client.temperatures
    db.collection_names()


def get_temp_logs(starting_date = datetime.datetime.utcnow() - datetime.timedelta(days=3), ending_date = datetime.datetime.utcnow() ):
    # default range is 3 days prior to now
    db = mon_client.temperatures
    return db.fermentation.find({"date": {"$lt": ending_date}, "date" : {"$gt": starting_date}}).sort("date")

def create_index():
    from pymongo import ASCENDING, DESCENDING
    db = mon_client.temperatures
    db.fermentation.create_index([("date", DESCENDING)])
