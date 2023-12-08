from flask import Flask, abort, make_response
import os
from datetime import datetime, timezone, timedelta
try:
    import zoneinfo
except ImportError:
    from backports import zoneinfo

zone = zoneinfo.ZoneInfo("Europe/Helsinki")
path = "data"

app=Flask(__name__) #instantiating flask object

@app.route('/cgi/price')
def func():
    now = datetime.now(timezone.utc)
    filename = now.strftime('%d%H') + '.price'
    filepath = os.path.join(path, filename)
    if os.path.exists(filepath):
        resp = make_response(open(filepath, "r").read())
        dt = (now.replace(minute=0, second=0, microsecond=0) + timedelta(hours=1)) - now
        resp.headers['Cache-Control'] = f'max-age={dt.seconds}'
        return resp
    else:
        abort(404)

application = app

if __name__=='__main__': #calling  main 
       app.debug=False #setting the debugging option for the application instance
       app.run() #launching the flask's integrated development webserver
