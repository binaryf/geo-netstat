# geo-netstat

*Where is my Mac connected to ?*

*What Application is using that connection ?*


Geo-Netstat is a program you run in your terminal and it provides geographical location for your application's internet connections.

![alt text](https://github.com/binaryf/geo-netstat/blob/master/GEO-NETSTAT.png)


Tested on MacOS Sierra.

This product includes GeoLite2 data created by MaxMind, available from
http://www.maxmind.com

#### How to compile libmaxminddb:
```
$ git clone --recursive https://github.com/maxmind/libmaxminddb
$ cd libmaxminddb
$ ./bootstrap
$ ./configure
$ make
$ make check
$ sudo make install
$ sudo ldconfig
```

#### How to install the databases:
```
Go to http://dev.maxmind.com/geoip/geoip2/geolite2/

Download the tar.gz's from the links:
'GeoLite2 City'
'GeoLite2 Country'
'GeoLite2 ASN'

Uncompress in a folder and copy the folder to a location.
The location is set in geo-netstat.h:

#define MMDB_ASN_DB      "/usr/local/db/GeoLite2-ASN.mmdb"
#define MMDB_CITY_DB     "/usr/local/db/GeoLite2-City.mmdb"
#define MMDB_COUNTRY_DB  "/usr/local/db/GeoLite2-Country.mmdb"
```

#### How to compile geo-netstat:
```
$ gcc -o geo-netstat -L/usr/local/lib/ -I/usr/local/include/ geo-netstat.c -lmaxminddb
```

#### How to install geo-netstat:
```
$ sudo mv geo-netstat /usr/local/bin
```

#### WebGL-Globe:

Geo-Netstat outputs a json datafile with latitude, longitude and magnitude
which can be loaded into WebGL-Globe.

See https://github.com/dataarts/webgl-globe

#### What can be improved (by you ?):

  - do fancy things with webgl-globe
  - add an update timer and refresh terminal 
  - support for IPv6
  - update data from the maxmind database
