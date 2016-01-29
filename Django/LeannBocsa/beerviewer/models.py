from django.db import models
import datetime

class ReadingsManager(models.Manager):
    def mod(self, start, end, modval):
        from django.db import connection
        cursor = connection.cursor()        
        cursor.execute("""
                       select timestamp, primary_temp, heater_state, cooling_state
                       FROM (
        select
        @row :=@row +1 AS rownum, timestamp, primary_temp, heater_state, cooling_state 
        from (
        select @row :=0) r, beerviewer_readings
        ) ranked
        where rownum %""" + str(modval) + """ = 1 AND timestamp BETWEEN '""" + str(start) + """' AND '""" + str(end) + """';""")

        return cursor.fetchall()

class Readings(models.Model):
    timestamp = models.DateTimeField(db_index=True, auto_now_add=True)
    primary_temp = models.DecimalField(verbose_name = 'primary temperature', decimal_places = 3, max_digits = 6)
    heater_state = models.BooleanField()
    cooling_state = models.BooleanField()
    objects = ReadingsManager()

    def Readings(TimeStamp, Primary, Heater, Cooling):
        self.timestamp = TimeStamp
        self.primary_temp = Primary
        self.heater_state = Heater
        self.cooling_state = Cooling

    def __unicode__(self):
        return "Time <%s>, Primary <%s>, Heating <%s>, Cooling <%s>" % (self.timestamp, self.primary_temp, self.heater_state, self.cooling_state)

    def howOld(self):
        return datetime.datetime.utcnow() - self.timestamp

class Beer(models.Model):
    start_time = models.DateTimeField()
    end_time = models.DateTimeField(null=True, blank=True)
    name = models.CharField('Beer Name', max_length=150)

    def __unicode__(self):
        retVal = self.name + " Startred on " + self.start_time.__str__()
        if(self.end_time):
            retVal = retVal + " ended on " + self.end_time.__str__()      
        return retVal

    

    #class Meta:
        #unique_together = (("start_time", "end_time", "name"),)

#Storing the temperature profile data into the database.  We have different ways of 
#using this data.  For example, we can have a beer that has the same temperature
#throughout fermentation.  For that, we would just have a link to the beer's 
#foreign key and a temperature in this table.  If we have multiple 
#time points for a changing beer, we can set the time the temperature
#takes affect by using the start_time column.  No start_time field means
#that the start_time is the beer's start_time
class TemperatureProfile(models.Model):
    beer = models.ForeignKey(Beer)
    type = models.PositiveSmallIntegerField('Type')
    temperature = models.DecimalField(verbose_name = 'Temperature', decimal_places = 3, max_digits = 6, null=True, blank=True)
    temperature_range = models.DecimalField(verbose_name = 'Temperature Range', decimal_places = 3, max_digits = 6, null=True, blank=True)
    start_time = models.DateTimeField(verbose_name = 'Start Time', null=True, blank=True, db_index=True)

    def TemperatureProfile(inBeer, inType, inTemperature=None, inTemperature_Range=None, inStart_Time=None):
        self.beer = inBeer
        self.type = inType
        self.temperature = inTemperature
        self.temperature_range = inTemperature_Range
        self.start_time = inStart_Time

class Color(models.Model):
    """
    The color's name (as used by the CSS 'color' attribute, meaning
    lowercase values are required), and a boolean of whether it's "liked"
    or not. There are NO USERS in this demo webapp, which is why there's no
    link/ManyToManyField to the User table.
 
    This implies that the website is only useable for ONE USER. If multiple
    users used it at the same time, they'd be all changing the same values
    (and would see each others' changes when they reload the page).
    """
    name = models.CharField(max_length=20)
    is_favorited = models.BooleanField(default=False)
 
    def __str__(self):
        return  self.name
 
    class Meta:
        ordering = ['name']
    




