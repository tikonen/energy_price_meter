#!/usr/bin/env python3

import os
from datetime import datetime, timedelta, timezone
import argparse
import isodate
from urllib.parse import urlencode
from urllib.request import urlopen
try:
    import zoneinfo
except ImportError:
    from backports import zoneinfo
try:
    from BeautifulSoup import BeautifulSoup
except ImportError:
    from bs4 import BeautifulSoup

# For full REST API documentation see
# https://transparency.entsoe.eu/content/static_content/Static%20content/web%20api/Guide.html
API_URL = 'https://web-api.tp.entsoe.eu/api'
EIC_code = '10YFI-1--------U'  # Finland

OUT_PATH = 'data'
LOG_PATH = 'log'

tzone = zoneinfo.ZoneInfo('Europe/Helsinki')

#  yyyyMMddHHmm


def periodStr(ts):
    return ts.strftime('%Y%m%d%H%M')

# Day ahead prices
# GET /api?documentType=A44&in_Domain=10YCZ-CEPS-----N&out_Domain=10YCZ-CEPS-----N&periodStart=201512312300&periodEnd=201612312300


def processxml(data, out_path, verbose):
    e = BeautifulSoup(data, 'xml').Publication_MarketDocument

    for timeseries in e.find_all('TimeSeries', recursive=False):
        interval = timeseries.Period.timeInterval
        period = timeseries.Period

        startdt = isodate.parse_datetime(interval.start.text)
        enddt = isodate.parse_datetime(interval.end.text)

        # print(interval.start.text)
        # print(interval.end.text)
        # print(period.resolution.text)

        # Should be ISO8601 interval. PT60M
        delta = isodate.parse_duration(period.resolution.text)

        prices = []

        for point in period.find_all('Point'):
            pos = int(point.position.text)
            price = point.find('price.amount').text
            pricepoint = (startdt + (pos - 1) * delta, float(price) / 10)
            prices.append(pricepoint)

        for pp in prices:
            dt = pp[0]
            price = pp[1]
            outpath = dt.strftime('%d%H') + '.price'
            outpath = os.path.join(out_path, outpath)
            with open(outpath, 'w') as f:
                f.write(f'{price:.2f}\n')


def test1():
    with open('test/debug-req1.xml', 'r') as f:
        data = f.read()
    processxml(data, '.', True)


def queryPrices(token, verbose: bool, log: bool):

    dt = datetime.utcnow().replace(hour=0, minute=0, second=0, microsecond=0)
    periodStart = dt
    periodEnd = dt + timedelta(hours=24)

    args = {
        'documentType': 'A44',
        'in_Domain': EIC_code,
        'out_Domain': EIC_code,
        'periodStart': periodStr(periodStart),
        'periodEnd': periodStr(periodEnd)
    }
    args['securityToken'] = token
    params = urlencode(args)
    url = f'{API_URL}?{params}'
    if verbose:
        print("Query day-ahead prices", url)

    req = urlopen(url)
    # data = req.read().decode(req.headers.get_content_charset())
    data = req.read().decode('utf-8')
    if verbose:
        print(data)
    if log:
        logfile = os.path.join(LOG_PATH, f'debug-req-{periodStr(periodStart)}')
        with open(logfile + '.xml', 'w') as f:
            f.write(data)
        with open(logfile + '.url', 'w') as f:
            f.write(url)
    return data


def main():
    # get default token from environment variable for developer convenience
    envtoken = os.environ.get('ENTSOE_TOKEN', '')

    parser = argparse.ArgumentParser(description='ENTSO-E Data Fetcher')
    parser.add_argument('--verbose', action='store_true', help='verbose mode')
    parser.add_argument('--log', action='store_true', help='Log data')
    parser.add_argument('--token', '-t', metavar='TOKEN',
                        default=envtoken, type=str, help='Authentication token')
    parser.add_argument('--out', metavar='PATH', type=str,
                        default=OUT_PATH, help='Output folder')
    args = parser.parse_args()

    # test1()
    # exit(1)
    if not args.token:
        print("ERROR: Authentication token required.")
        exit(1)

    if not os.path.exists(args.out):
        print(f'ERROR: Output path {args.out} does not exists')
        exit(1)

    xml = queryPrices(args.token, args.verbose, args.log)
    processxml(xml, args.out, args.verbose)


if __name__ == '__main__':
    main()
