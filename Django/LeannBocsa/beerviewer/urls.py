from django.conf.urls import url

from . import views

urlpatterns = [
    url(r'^status/$', views.status, name='status'),
    url(r'^$', views.IndexView.as_view(), name='index'),    
    url(r'^getLatestBeer$', views.getLatestBeer, name='getbeer'),
    url(r'^(?P<id>[0-9]+)/details/$', views.details, name='details'),
    url(r'^(?P<beerName>.*)/getBeer/$', views.beer, name='getSpecificBeer'),
    url(r'^(?P<beerName>.*)/beerDetailChooser/$', views.beerDetailChooser, name='beerDetailChooser'),
    url(r'^(?P<beerName>.*)/feed/$', views.feed, name='feed'),
]