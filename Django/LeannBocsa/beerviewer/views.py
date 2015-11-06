#from django.shortcuts import render
from django.core.exceptions import *
import MySQLdb
#from reader import models
from .models import Readings, Beer, TemperatureProfile
from .forms import GranularityForm
import datetime
import time
#from datetime import datetime, timedelta

# Create your views here.
import os
from decimal import Decimal
from datetime import timedelta
os.environ['MPLCONFIGDIR'] = '/home/sean/data'
from django.http import HttpResponse
#import numpy as np  # (*) numpy for math functions and arrays
import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as plt
#import cStringIO
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure

from django.template import RequestContext, loader
from django.shortcuts import render, get_object_or_404, render_to_response
from django.http import Http404
from django.views import generic
from django.utils import timezone

class IndexView(generic.ListView):
    template_name = 'beerviewer/index.html'
    context_object_name = 'reading_list'
    

    def get_queryset(self):
        return Beer.objects.order_by('-id')


#def index(request):    
#    reading_list = Beer.objects.order_by('-id')   
#    context = RequestContext(request, {
#        'reading_list': reading_list,
#    })
#    return render(request, 'reader/index.html', context)
records_per_minute = 12
#def status(request):
#    f = open("/home/pi/data/readings.csv", 'r')    
#    return HttpResponse("The data is : " +  "<br />".join(f.read().split("\n")))
    #return HttpResponse("You got to the status data bro!")

def getLatestBeer(request):
    data = Readings.objects.order_by('-id')[:5]
    output = ','.join([str(p.id) for p in data])
    return HttpResponse(output)

def details(request, id):     
    try:
        db = MySQLdb.connect(host="localhost", # your host, usually localhost
                                user="root", # your username
                                passwd="unbr34k4b73", # your password
                                db="testdb") # name of the data base

        # you must create a Cursor object. It will let
        #  you execute all the queries you need
        cur = db.cursor() 
        cur.execute("SELECT * from reader_readings where id = " + id)
        row = cur.fetchone()
        response = HttpResponse("For ID " + str(id) + " the results are " + str(row[0]) + "," + str(row[1]) + "," + str(row[2]) + "," + str(row[3]) + "," + str(row[4]))
        cur.close()
        db.close()
        return response;
    except:
        raise Http404("ID" + str(id) + " does not exist")             

def beerDetailChooser(request, beerName):
    beer = ""
    output = ""
    try:
        beer = Beer.objects.get(name=beerName)
    except Beer.DoesNotExist:
        output = "No beer " + beerName + " found";
    except MultipleObjectsReturned:
        output = "More than one beer " + beerName + " found";
    if(beer):
        output = "Found a beer by " + beerName
        
        output = output + " And granularity was "
        if request.method == 'POST':
        # create a form instance and populate it with data from the request:
            form = GranularityForm(request.POST)
            # check whether it's valid:
            if form.is_valid():
                # process the data in form.cleaned_data as required
                # ...
                # redirect to a new URL:
                output = output + " Successfully got granularity for real of " + str(form.cleaned_data['granularity'])
                return generateGraphWithRange(beer, 
                                             form.cleaned_data['granularity'], 
                                             form.cleaned_data['start_time'],
                                             form.cleaned_data['end_time'])

    # if a GET (or any other method) we'll create a blank form
        else:
            end_time = beer.end_time
            if not end_time:
                #Not defined, use right now
                end_time = datetime.datetime.utcnow()
            form = GranularityForm(initial={'end_time' : end_time,
                                            'start_time' : beer.start_time})

    #return render(request, 'name.html', {'form': form})
        #Now that we have a valid beer object, get the start and end date
        #return generateGraph(beer, granularity, 65, 68)
    context = RequestContext(request, {
        'beer': beer,
        'form': form,
    })
    return render(request, 'beerviewer/detailchooser.html', context)

def feed(request, beerName):
    return render(request, 'beerviewer/feed.html')

def generateGraphWithRange(beer, 
                           granularity, 
                           start, 
                           end,
                           xwidth=20,
                           ywidth=12):
    records = Readings.objects.mod(start, end, records_per_minute*granularity)
    #output = "Found " + str(records.count()) + " records in the table"
    dates  = []        
    primarytemp = []
    mintemp = []
    maxtemp = []
    heat_start = -1
    cooling_start = -1
    
    fig = plt.figure(figsize=(xwidth,ywidth), dpi=80)
    ax = fig.add_subplot(111)
    colorings = []
    for curt in records:
        dates.append(curt[0])
        primarytemp.append(curt[1])
        if(curt[2] == 1 and heat_start == -1):
            #Heating state turned on
            heat_start = curt[0]
        elif(heat_start != -1 and curt[2] == 0):
            #Heating switched off
            #plt.axhspan(heat_start, curt[0], facecolor='r', alpha=0.5)
            colorings.append([matplotlib.dates.date2num(heat_start), matplotlib.dates.date2num(curt[0]), 'r', .5])
            heat_start = -1;

        #Check cooling
        if(curt[3] == 1 and cooling_start == -1):
            #Cooling state turned on
            cooling_start = curt[0]
        elif(cooling_start != -1 and curt[3] == 0):
            #Cooling turned off
            #plt.axhspan(cooling_start, curt[0], facecolor='b', alpha=0.5)
            colorings.append([matplotlib.dates.date2num(cooling_start), matplotlib.dates.date2num(curt[0]), 'b', .5])
            cooling_start = -1;
        #mintemp.append(min)
        #maxtemp.append(max)
    #See if we have an ongoing event
    if(cooling_start != -1):
            #Append a new end
            colorings.append([matplotlib.dates.date2num(cooling_start), matplotlib.dates.date2num(end), 'b', .5])
    if(heat_start != -1):
            colorings.append([matplotlib.dates.date2num(heat_start), matplotlib.dates.date2num(end), 'r', .5])
    m_dates = matplotlib.dates.date2num(dates)
    ax.plot_date(m_dates, primarytemp, 'b-', label="Internal")        
    #Colorize our graph data with pretty bars
    for row in colorings:
        plt.axvspan(row[0], row[1], facecolor=row[2], alpha=row[3])
    
    #Plot in our ideal temperatures, if we have one
    if(beer):
        records = TemperatureProfile.objects.filter(beer=beer)
        dates = []
        ideal = []

        for curt in records:
            #Go through and pull out or times and temps
            time = curt.start_time
            if(time):
                #We have a starting point
                #TODO: Someone could put a wrong time in here before our graph started
                dates.append(matplotlib.dates.date2num(time))
            else:
                #Don't, have to use the starting point of the beer
                dates.append(m_dates[0])
            ideal.append(curt.temperature)
        if(dates and ideal):
            #Put in the last end point so we can draw our lines
            dates.append(m_dates[-1])
            ideal.append(ideal[-1])
            ax.plot_date(dates, ideal, 'r--', label="Target")

    #Put in our legend
    #red_patch = mpatches.Patch(color='red', label='The red data')
    #plt.legend(handles=[red_patch])
    handles, labels = ax.get_legend_handles_labels()
    ax.legend(handles, labels, bbox_to_anchor=(1.00, 1), loc=2, borderaxespad=0.)
    #ax.plot_date(m_dates, maxtemp, 'r--')

    #Set the labels
    ax.set_xlabel('Date/Time')
    ax.set_ylabel(u"\u00B0" + " Farenheit")
    ax.autoscale_view()
    fig.autofmt_xdate()
    canvas=FigureCanvas(fig)
    response= HttpResponse(content_type='image/png')
    canvas.print_png(response)
    # delete figure to prevent memory leaks!
    fig.clf()
    plt.close()
    return response

#Generate a graph for the whole beer time
def generateGraph(beer, minutes):
    #Now that we have a valid beer object, get the start and end date
    start = beer.start_time;
    end = beer.end_time;
    
    if end is None:
        #No end date.  Use the current time
        end = datetime.datetime.utcnow()
    #Now search through and find our records from this beer
    #They come in the format of timestamp, primary_temp, heater_state, cooling_state
    return generateGraphWithRange(beer, minutes, start, end)
    

def beer(request, beerName):
    beer = ""
    output = ""
    try:
        beer = Beer.objects.get(name=beerName)
    except Beer.DoesNotExist:
        output = "No beer " + beerName + " found";
    except MultipleObjectsReturned:
        output = "More than one beer " + beerName + " found";
    if(beer):
        output = "Found a beer by " + beerName
        #Now that we have a valid beer object, get the start and end date
        return generateGraph(beer, 5)      
    return HttpResponse(output)

from django.shortcuts     import redirect
from django.views.generic import ListView
from beerviewer.models   import Color
 
MIN_SEARCH_CHARS = 2


class ColorList(ListView):
    """
    Displays all colors in a table with only two columns: the name
    of the color, and a "like/unlike" button.
    """
    model = Color
    context_object_name = "colors"
    template_name = 'beerviewer/color_list.html'    

    def dispatch(self, request, *args, **kwargs):
        return super(ColorList, self).dispatch(request, *args, **kwargs)
 
    def get_queryset(self):
        """
        This returns the all colors, for display in the main table.
 
        The search result query set, if any, is passed as context.
        """
        return  super(ColorList, self).get_queryset()
 
    def get_context_data(self, **kwargs):
        #Get the current context.
        context = super(ColorList, self).get_context_data(**kwargs)
 
        context["MIN_SEARCH_CHARS"] = MIN_SEARCH_CHARS
 
        return  context
 
def submit_color_search_from_ajax(request):
    """
    Processes a search request, ignoring any where less than two
    characters are provided. The search text is both trimmed and
    lower-cased.
 
    See <link to MIN_SEARCH_CHARS>
    """
 
    colors = []  #Assume no results.
 
    global  MIN_SEARCH_CHARS
 
    search_text = ""   #Assume no search
    if(request.method == "GET"):
        search_text = request.GET.get("color_search_text", "").strip().lower()
        if(len(search_text) < MIN_SEARCH_CHARS):
            """
            Ignore the search. This is also validated by
            JavaScript, and should never reach here, but remains
            as prevention.
            """
            search_text = ""
 
    #Assume no results.
    #Use an empty list instead of None. In the template, use
    #   {% if color_search_results.count > 0 %}
    color_results = []
 
    if(search_text != ""):
        color_results = Color.objects.filter(name__contains=search_text)
 
    #print('search_text="' + search_text + '", results=' + str(color_results))
 
    context = {
        "search_text": search_text,
        "color_search_results": color_results,
        "MIN_SEARCH_CHARS": MIN_SEARCH_CHARS,
    };
 
    return  render_to_response("beerviewer/color_search_results__html_snippet.txt",
                               context)

def toggle_color_like(request, color_id):
    """Toggle "like" for a single color, then refresh the color-list page."""
    color = None
    try:
        #There's only one object with this id, but this returns a list
        #of length one. Get the first (index 0)
        color = Color.objects.filter(id=color_id)[0]
    except Color.DoesNotExist as e:
        raise  ValueError("Unknown color.id=" + str(color_id) + ". Original error: " + str(e))
 
    #print("pre-toggle:  color_id=" + str(color_id) + ", color.is_favorited=" + str(color.is_favorited) + "")
 
    color.is_favorited = not color.is_favorited
    color.save()  #Commit the change to the database
 
    #print("post-toggle: color_id=" + str(color_id) + ", color.is_favorited=" + str(color.is_favorited) + "")
 
    #return  redirect("beerviewer:color_list")  #See urls.py
    return  render_to_response("beerviewer/color_like_link__html_snippet.txt",
                            {"color": color})

def bootstrap(request):
    return render_to_response("beerviewer/bootstrap.html")

def simple(request):
    return generateGraphWithRange("", 
                                  5, 
                                  (datetime.datetime.utcnow() - timedelta(days=1)), datetime.datetime.utcnow(), 
                                  xwidth=10, 
                                  ywidth=6);

def status(request):
    beernames = []
    #Get the names of our currently running beers
    for beer in Beer.objects.filter(end_time__isnull=True):
        beernames.append(str(beer.name))

    #See if we're still getting updates
    latest = Readings.objects.latest('timestamp')   
    timeSinceLastUpdate = 0 
    if(latest):        
        #We don't care about microseconds.  Just get the actual integer seconds
        timeSinceLastUpdate = int((datetime.datetime.utcnow() - latest.timestamp.replace(tzinfo=None)).total_seconds())                
    
    #Get a minigraph of the last day of data
    records = Readings.objects.mod((datetime.datetime.utcnow() - timedelta(days=1)),
                                   datetime.datetime.utcnow(), 
                                   5)
    recordData = [];
    for curt in records:
        #timestamp = (curt[0] - datetime.datetime(1970, 1, 1)).total_seconds()
        recordData.append([time.mktime(curt[0].timetuple()) * 1000, str(curt[1])]);
    
    data = {"data": recordData, 'label': 'Temperature'};
  
        
    #Get the current heater/cooling status and how long it's been on
    timeChanged = getCurrentOperation(latest)

    context = {
        "beers": beernames,
        "series_json": data,
        "last_update": timeSinceLastUpdate,
        };
    context.update(timeChanged)
    return render_to_response("beerviewer/status.html", context)

#Look at what's currently happening with the heating and cooling, and determine how
#long it's been doing it for
#<param name="latest">The most recent row in the readings table</param>
#TODO: Don't look at every record, look at every 20th record or something
def getCurrentOperation(latest):
    #Find out what's the current state of things by getting the most recent
    #temperature reading
    

    
    records = Readings.objects.order_by('-timestamp')[:records_per_minute*60*24]
    changedTime = ''
    retVal = {};
    
    if(latest.heater_state == 1):
        #Heating on, build the last 4 hours of heating data        
        retVal['State'] = "Heating"
        for curt in records:
            #Search for the last change
            if(curt.heater_state == 0):
                #Found it
                changedTime = curt.timestamp
                break
    elif(latest.cooling_state == 1):
        #Cooling on, build the last 4 hours of cooling data
        retVal['State'] = "Cooling" 
        for curt in records:
           #Search for the last change
           if(curt.cooling_state == 0):
                #Found it
                #changedTime = time.gmtime(time.mktime(curt.timestamp.timetuple()) * 1000)
                changedTime = curt.timestamp
                break
    else:
        #Neither, sitting idle.  Build the last 4 hours of idle data
        retVal['State'] = "Stable"
        for curt in records:        
            #Search for the last change
            if(curt.cooling_state == 1 or curt.heater_state == 1):
                #Found it
                #changedTime = time.gmtime(time.mktime(curt.timestamp.timetuple()) * 1000)        
                changedTime = curt.timestamp
                break
    
    if(changedTime):
        #We found a time
        timedelta = datetime.datetime.utcnow() - changedTime.replace(tzinfo=None)
        if(timedelta.seconds // 3600 > 0):
            retVal['Time'] = "%d hours, %d minutes" % (timedelta.seconds // 3600, (timedelta.seconds // 60) % 60)
        else:
            retVal['Time']= "%d minutes" % ((timedelta.seconds // 60) % 60)        
    else:
        retVal['Time'] = "over 24 hours"

    #Set what the current temperature is
    if(latest):
        retVal['Current_Temperature'] = latest.primary_temp
    
    return retVal;

def flot_test(request):
    #Now that we have a valid beer object, get the start and end date
    beer = ""
    output = ""
    beerName = "Ol' Rasputin"
    try:
        beer = Beer.objects.get(name=beerName)
    except Beer.DoesNotExist:
        output = "No beer " + beerName + " found";
    except MultipleObjectsReturned:
        output = "More than one beer " + beerName + " found";
    if(beer):
        output = "Found a beer by " + beerName
        #Now that we have a valid beer object, get the start and end date   
    start = beer.start_time;
    end = beer.end_time;
    
    if end is None:
        #No end date.  Use the current time
        end = datetime.datetime.utcnow()
    records = Readings.objects.mod(start, end, records_per_minute*5)
    recordData = [];
    for curt in records:
        #timestamp = (curt[0] - datetime.datetime(1970, 1, 1)).total_seconds()
        recordData.append([time.mktime(curt[0].timetuple()) * 1000, str(curt[1])]);
        
    data = [{"data":recordData}];
    #options = {
    #    series: {stack: 0,
    #             lines: {show: false, steps: false },
    #             bars: {show: true, barWidth: 0.9, align: 'center',},},
    #    xaxis: {ticks: [[1,'One'], [2,'Two'], [3,'Three'], [4,'Four'], [5,'Five']]},
    #};
    context = {
        "series_json": data,
     #   "options": options,
        };
    return  render_to_response("beerviewer/flot_test.html",
                               context)
