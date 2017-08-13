# geo-netstat

*Where is my Mac connected to ?*

*What Application is using that connection ?*


Geo-Netstat is a program you run in your terminal and it provides geographical location to your application's internet connections.

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

#### How to compile geo-netstat:
```
$ gcc -o geo-netstat -L/usr/local/lib/ -I/usr/local/include/ geo-netstat.c -lmaxminddb
```

#### How to install geo-netstat:
```
$ sudo mv geo-netstat /usr/local/bin
```

#### How to run webgl-globe:
(https://hub.docker.com/help/)
```
$ cd geo-netstat
$ git clone https://github.com/dataarts/webgl-globe.git
$ docker-compose up -d
```

#### What can be improved (by you ?):

  - do fancy things with webgl-globe
  - support for IPv6
  - update data from the maxmind database
