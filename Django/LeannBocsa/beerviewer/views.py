#from django.shortcuts import render
from django.core.exceptions import *
import MySQLdb
#from reader import models
from .models import Readings, Beer, TemperatureProfile
from .forms import GranularityForm
import datetime

# Create your views here.
import os
os.environ['MPLCONFIGDIR'] = '/home/pi/data'
from django.http import HttpResponse
#import numpy as np  # (*) numpy for math functions and arrays
import matplotlib

from matplotlib import pyplot as plt
#import cStringIO
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure

from django.template import RequestContext, loader
from django.shortcuts import render, get_object_or_404
from django.http import Http404
from django.views import generic

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

def status(request):
    f = open("/home/pi/data/readings.csv", 'r')    
    return HttpResponse("The data is : " +  "<br />".join(f.read().split("\n")))
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
                end_time = datetime.datetime.utcnow            
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

def generateGraphWithRange(beer, minutes, start, end):
    records = Readings.objects.mod(start, end, 12*minutes)
    #output = "Found " + str(records.count()) + " records in the table"
    dates  = []        
    primarytemp = []
    mintemp = []
    maxtemp = []
    heat_start = -1
    cooling_start = -1
    
    fig = plt.figure(figsize=(20,12), dpi=80)
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
    
    m_dates = matplotlib.dates.date2num(dates)
    ax.plot_date(m_dates, primarytemp, 'b-', label="Internal")        
    #Colorize our graph data with pretty bars
    for row in colorings:
        plt.axvspan(row[0], row[1], facecolor=row[2], alpha=row[3])
    
    #Plot in our ideal temperatures
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
        #sio = cStringIO.StringIO()
        #fig.savefig(sio, format='png')
        # delete figure to prevent memory leaks!
        #fig.clf()
        #plt.close()
        #return HttpResponse(sio.getvalue(), content_type="image/png")
    return HttpResponse(output)