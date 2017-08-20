/*

  geo-netstat.h : header file for Geo-Netstat.

  updated: 2017-08-13

*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <maxminddb.h>
#include <fcntl.h>

// MAX NUMBER OF SIMULTANEOUS CONNECTION TO MONITOR
#define MAX_CONNECTIONS  1000

// THE DEFAULT MAGNITUDE PER CONNECTION AT A LOCATION (FOR WEBGL-GLOBE)
#define MAGNITUDE_VALUE_PER_LOC 0.01

// DATABASES
#define MMDB_ASN_DB      "/usr/local/db/GeoLite2-ASN.mmdb"
#define MMDB_CITY_DB     "/usr/local/db/GeoLite2-City.mmdb"
#define MMDB_COUNTRY_DB  "/usr/local/db/GeoLite2-Country.mmdb"

// MMDB QUERY OPTIONS
#define MMDB_DUMP_ALL     0
#define MMDB_ASN          1
#define MMDB_COUNTRY      2
#define MMDB_LATITUDE     3
#define MMDB_LONGITUDE    4

// TEMP FILES (USED TO COLLECT OUTPUT FROM NETSTAT AND PS)
#define FILE_NETSTAT "/tmp/.netstat"
#define FILE_PS      "/tmp/.ps"

// FILENAME AND PATH FOR JSON OUTPUT (FOR WEBGL-GLOBE)
#define FILE_JSON    "/tmp/data.json"

// NETSTAT CMDLINE (OSX)
#define NETSTAT_COM_IP4 "netstat -n -f inet -p tcp -v >/tmp/.netstat"

// PS CMDLINE (OSX)
#define PS_PROC_COMM    "/bin/ps -p %s -o comm >/tmp/.ps"

// DATA STRUCTURE
// NOTE: WE ARE USING SRTINGS AS DATATYPE BECAUSE WE ARE WRITING TO TERMINAL AND FILE
struct net_info {

  // NAME OF EXECUTABLE
  char executable[1024];

  // HOSTNAME, AS AND COUNTRY
  char hostname[256];
  char as[64];
  char country[64];

  // GEO LOCATION AND MANGITUDE
  char latitude[32];
  char longitude[32];
  char magnitude[32];

  // PLOT LOCATION YES / NO
  int plot;

  // NETSTAT OUTPUT
  char proto[6];
  char recv_q[16];
  char send_q[16];
  char local_ip[21];
  char local_port[6];
  char foreign_ip[21];
  char foreign_port[6];
  char state[32];
  char rhiwat[8];
  char shiwat[8];
  char pid[8];
  char epid[8];
};

char *get_next(char *src, char *dest);
char *get_mmdb_value(char *database, char *ip_address, int query);
