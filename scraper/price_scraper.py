#!/usr/bin/env python3

# Read a HTML site and attempt find and print out an specific list item value

import gzip
from urllib.request import Request, urlopen
from datetime import datetime

try: 
    from BeautifulSoup import BeautifulSoup
except ImportError:
    from bs4 import BeautifulSoup



def scrape_data(html):

    now = datetime.now()
    hs = now.hour
    he = hs + 1
    if he > 23:
        he = 0

    # Search string is an 24 clock hour range '09 - 10'
    hourstr = str(hs).rjust(2, '0') + " - " + str(he).rjust(2, '0')
    price = '0'

    e = BeautifulSoup(html, features='html.parser')
    ul = e.body.find('div', id='today-wrapper').find('ul', class_='spotprice-list mt-3')
    for li in ul('li'):
        span = li.find('span')
        if span and span.text ==  hourstr:
            price = li.find('pricedata').text.strip()
            break

    return price


headers = {
    'sec-ch-ua': '"Not?A_Brand";v="8", "Chromium";v="108", "Google Chrome";v="108"',
    'sec-ch-ua-mobile': '?0',
    'sec-ch-ua-platform': '"Windows"',
    'DNT': '1',
    'Upgrade-Insecure-Requests': '1',
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36',
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9',
    'Sec-Fetch-Site': 'none',
    'Sec-Fetch-Mode': 'navigate',
    'Sec-Fetch-User': '?1',
    'Sec-Fetch-Dest': 'document',
    'Accept-Encoding': 'gzip, deflate, br',
    'Accept-Language': 'en-US,en;q=0.9,fi;q=0.8',
    'sec-gpc': '1'
}

URL='https://www.herrfors.fi/fi/spot-hinnat/'

#html = open("testdata/herrfors.fi-response.txt", "r", encoding='Latin1').read()

req = Request(URL, None, headers)
resp = urlopen(req)

#print(resp.info())

if resp.info().get('Content-Encoding') == 'gzip':
    f = gzip.GzipFile(fileobj=resp)
    data = f.read()
else:
    data = resp.read()
html = data.decode('utf-8')

price = scrape_data(html)
print("Price is %s" % price)
open('spot', "w", encoding='Latin1').write(price)

