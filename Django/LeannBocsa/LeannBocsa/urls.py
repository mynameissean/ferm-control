"""
Definition of urls for Django_Unchained.
"""

from datetime import datetime
from django.conf.urls import patterns, url
#from app.forms import BootstrapAuthenticationForm

# Uncomment the next lines to enable the admin:
from django.conf.urls import include
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
    # Examples:
    #url(r'^$', 'app.views.home', name='home'),
    #url(r'^contact$', 'app.views.contact', name='contact'),
    #url(r'^about', 'app.views.about', name='about'),
    #url(r'^login/$',
    #    'django.contrib.auth.views.login',
    #    {
    #        'template_name': 'app/login.html',
    #        'authentication_form': BootstrapAuthenticationForm,
    #        'extra_context':
    #        {
    #            'title':'Log in',
    #            'year':datetime.now().year,
    #        }
    #    },
    #    name='login'),
    url(r'^admin/', include(admin.site.urls)),
    url(r'^reader/', include('beerviewer.urls', namespace="beerviewer")),
    url(r'^logout$',
        'django.contrib.auth.views.logout',
        {
            'next_page': '/',
        },
        name='logout'),

    # Uncomment the admin/doc line below to enable admin documentation:
    # url(r'^admin/doc/', include('django.contrib.admindocs.urls')),

    # Uncomment the next line to enable the admin:
    
)

#if settings.DEBUG:
#    import debug_toolbar
#    urlpatterns += patterns('',
#        url(r'^debug/', include(debug_toolbar.urls)),
#    )
