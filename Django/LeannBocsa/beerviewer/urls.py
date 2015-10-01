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
    url(r"^color$", views.ColorList.as_view(), name="color_list"),
    url(r"^like_color_(?P<color_id>\d+)/$", "beerviewer.views.toggle_color_like", name="toggle_color_like"),
    url(r"^search/$", "beerviewer.views.submit_color_search_from_ajax", name="color_list"),
    url(r"^bootstrap/$", views.bootstrap, name="bootstrap"),
    url(r"^flot_test/$", views.flot_test, name="flot_test"),
    url(r"^status/$", views.status, name="status"),
    url(r"^status/simple.png$", views.simple, name="simple"),
]