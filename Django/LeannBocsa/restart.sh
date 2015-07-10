sudo supervisorctl stop gunicorn 
sudo service nginx stop
sudo supervisorctl start gunicorn
sudo service nginx start

