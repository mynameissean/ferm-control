from django import forms

class GranularityForm(forms.Form):
    granularity = forms.IntegerField(label='Granularity',min_value=5, max_value=100, initial=10)
    start_time = forms.DateTimeField(label='Starting Time')
    end_time = forms.DateTimeField(label='Ending Time')