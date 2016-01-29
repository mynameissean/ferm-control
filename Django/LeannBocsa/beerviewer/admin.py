from django.contrib import admin

# Register your models here.
from beerviewer import models

class ReadingsAdmin(admin.ModelAdmin):
    fieldsets = [
        (None, {'fields':['primary_temp']}),
        ('Date Information', {'fields':['timestamp'], 'classes': ['collapse']}),
        ('States',{'fields':['heater_state', 'cooling_state']})
        ]

class BeerAdmin(admin.ModelAdmin):
    fieldsets = [
        ('Beer Name', {'fields':['name']}),
        ('Start Time', {'fields':['start_time']}),
        ('End Time', {'fields':['end_time']})
        ]
class TemperatureProfileAdmin(admin.ModelAdmin):
    list_display = ('beer_name', 'start_time', 'temperature' )
    #fieldsets = [
    #    ('beer', 'beer_name'),
    #    ('Type', {'fields':['type']}),
    #    ('Temperature', {'fields':['temperature']}),
    #    ('Temperature Range', {'fields':['temperature_range']}),
    #    ('Start Time', {'fields':['start_time']}),
    #    ]

    def beer_name(self, instance):
        return instance.beer.name

admin.site.register(models.Readings, ReadingsAdmin)
admin.site.register(models.Beer, BeerAdmin)
admin.site.register(models.TemperatureProfile, TemperatureProfileAdmin)
admin.site.register(models.Color);