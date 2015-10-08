# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('beerviewer', '0002_color'),
    ]

    operations = [
        migrations.AlterField(
            model_name='readings',
            name='timestamp',
            field=models.DateTimeField(auto_now_add=True, db_index=True),
        ),
    ]
