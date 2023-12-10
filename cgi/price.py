from flask import Flask, abort, make_response
import os
from datetime import datetime, timezone, timedelta
try:
    import zoneinfo
except ImportError:
    from backports import zoneinfo

zone = zoneinfo.ZoneInfo("Europe/Helsinki")
path = "data"

app = Flask(__name__)  # instantiating flask object

# Price request
#
# Return is a list of hourly prices from or up to now, depending if offset is negative or positive
#
# Response Format:
#  <price> # <DDhh>
#
#  <price> - eurocents per kWh. "N/A" if not available yet.
#  <DDhh>  - Day of month and hour in UTC
#
# Examples. Here UTC time is 09:00 on 4th day of month
#
# /cgi/price
#  11.31 # 0409
#
# /cgi/price/-1
#  12.32 # 0408
#  11.31 # 0409
#
# /cgi/price/2
#  12.32 # 0409
#  11.31 # 0410
#  N/A # 0411
#


@app.route('/cgi/price')
@app.route('/cgi/price/<offset>')
def func(offset=0):
    # input validation
    try:
        offset = int(offset)
    except:
        abort(404)
    if offset > 12:
        abort(404)
    if offset < -12:
        abort(404)

    # define hour range to fetch
    now = datetime.now(timezone.utc)
    max_age = ((now.replace(minute=0, second=0, microsecond=0) +
               timedelta(hours=1)) - now).seconds
    start = 0 if offset > 0 else offset
    end = 0 if offset < 0 else offset

    lines = []
    for offset in range(start, end+1):
        # read price from each hour file
        dt = now + timedelta(hours=offset)
        filename = dt.strftime('%d%H') + '.price'
        filepath = os.path.join(path, filename)
        line = "N/A"
        if os.path.exists(filepath):
            line = open(filepath, "r").read().strip()
        lines.append(line + ' #' + dt.strftime('%d%H'))

    # join results together and return with a response
    resp = make_response('\n'.join(lines) + '\n')
    resp.headers['Cache-Control'] = f'max-age={max_age}'
    resp.headers['Content-Type'] = 'text/plain; charset=utf-8'
    return resp


application = app

if __name__ == '__main__':  # calling  main
    app.debug = True  # setting the debugging option for the application instance
    app.run()  # launching the flask's integrated development webserver
