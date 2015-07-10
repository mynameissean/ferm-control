# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Beer',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('start_time', models.DateTimeField()),
                ('end_time', models.DateTimeField(null=True, blank=True)),
                ('name', models.CharField(max_length=150, verbose_name=b'Beer Name')),
            ],
        ),
        migrations.CreateModel(
            name='Readings',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('timestamp', models.DateTimeField(db_index=True)),
                ('primary_temp', models.DecimalField(verbose_name=b'primary temperature', max_digits=6, decimal_places=3)),
                ('heater_state', models.BooleanField()),
                ('cooling_state', models.BooleanField()),
            ],
        ),
        migrations.CreateModel(
            name='TemperatureProfile',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('type', models.PositiveSmallIntegerField(verbose_name=b'Type')),
                ('temperature', models.DecimalField(null=True, verbose_name=b'Temperature', max_digits=6, decimal_places=3, blank=True)),
                ('temperature_range', models.DecimalField(null=True, verbose_name=b'Temperature Range', max_digits=6, decimal_places=3, blank=True)),
                ('start_time', models.DateTimeField(db_index=True, null=True, verbose_name=b'Start Time', blank=True)),
                ('beer', models.ForeignKey(to='beerviewer.Beer')),
            ],
        ),
    ]
