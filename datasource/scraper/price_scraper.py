#!/usr/bin/env python3

# Read a HTML site and attempt find and print out an specific list item value

import re
import os
import argparse
import gzip
from urllib.request import Request, urlopen
from datetime import datetime

try: 
    from BeautifulSoup import BeautifulSoup
except ImportError:
    from bs4 import BeautifulSoup

def scrape_current_price(html):

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

def scrape_todays_prices(html):

    prices = []
    hrex = re.compile(r'^(\d\d)[^\d]+(\d\d)$')

    e = BeautifulSoup(html, features='html.parser')
    ul = e.body.find('div', id='today-wrapper').find('ul', class_='spotprice-list mt-3')
    for li in ul('li'):
        span = li.find('span')
        if not span:
            continue
        m = hrex.match(span.text)
        if m:
            price = li.find('pricedata').text.strip()
            prices.append((m[1], price))

    return prices


# Chrome headers
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

def parse_test(filename):
    print(scrape_current_price(html))
    print(scrape_todays_prices(html))

def load_html(url, verbose):
    if verbose:
        print("Requesting", url)

    req = Request(url, None, headers)
    resp = urlopen(req)

    if verbose:
        print(resp.info())

    if resp.info().get('Content-Encoding') == 'gzip':
        f = gzip.GzipFile(fileobj=resp)
        data = f.read()
    else:
        data = resp.read()
    html = data.decode('utf-8')
    return html

def main():
    parser = argparse.ArgumentParser(description='Scraper')
    parser.add_argument('--verbose', action='store_true', help='verbose mode')
    parser.add_argument('--out', metavar='OUT', type=str, help='Output folder')
    parser.add_argument("url", metavar='URL', type=str, help='URL to scrape')
    args = parser.parse_args()

    html = load_html(args.url, args.verbose)
    #html = open("testdata/response.txt", "r", encoding='Latin1').read()

    price = scrape_current_price(html)
    print("Current price is %s" % price)

    outputpath = args.out if args.out else '';
    outputfile = os.path.join(outputpath, 'spot')
    if args.verbose:
        print("Writing to ", outputfile)
    open(outputfile, "w", encoding='Latin1').write(price)

    prices = scrape_todays_prices(html)
    for p in prices:
        outputfile = os.path.join(outputpath, p[0]+'-spot')
        if args.verbose:
            print("Writing to ", outputfile)
        open(outputfile, "w", encoding='Latin1').write(p[1])

if __name__ == '__main__':
    main()
