/*

   geo-netstat.c : source file for Geo-Netstat.
   Updated : 2017-08-19

   https://github.com/binaryf/geo-netstat

   This product includes GeoLite2 data created by MaxMind, available from
   http://www.maxmind.com

 */

#include "geo-netstat.h"

int main(int argc, char **argv) {

  // execute netstat and store output to file
  if(system(NETSTAT_COM_IP4) == -1) {
    perror(NETSTAT_COM_IP4);
    exit(EXIT_FAILURE);
  }

  // find the size
  struct stat st_buf;
  if(lstat(FILE_NETSTAT,&st_buf) == -1) {
    perror(FILE_NETSTAT);
    exit(EXIT_FAILURE);
  }
  size_t size;
  size = (size_t)st_buf.st_size;

  // allocate the buffer
  char *buf;
  buf = malloc(size);
  if(buf == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  // read netstat output from file
  int fd;
  fd = open(FILE_NETSTAT,O_RDONLY);
  if(fd == -1) {
    perror(FILE_NETSTAT);
    exit(EXIT_FAILURE);
  }
  if(read(fd,buf,size) == -1) {
    perror(FILE_NETSTAT);
    exit(EXIT_FAILURE);
  }
  if(close(fd) == -1) {
    perror(FILE_NETSTAT);
    exit(EXIT_FAILURE);
  }

  // make a pointer to buffer
  char *ptr;
  ptr = &buf[0];

  // number of lines of data
  int num_lines;
  num_lines = 0;

  // calculate number of lines and adding string termination to the line
  while((ptr = index(ptr,0x0a))) {
    ptr[0] = 0;
    ++num_lines;
    ++ptr;
  }

  // add offset to pointer to skip the first line
  ptr = &buf[0];
  ptr += strlen(ptr)+1;

  // decrement number of lines to skip the two first lines
  num_lines -= 2;

  // create the data structure
  struct net_info connections[MAX_CONNECTIONS];

  for (int i=0; i<num_lines && i<MAX_CONNECTIONS; i++) {

    // add offset to pointer for the next line
    ptr += strlen(ptr)+1;

    // clear the memory
    memset(&connections[i],0,sizeof(struct net_info));

    // get space separated values
    ptr = get_next(ptr,connections[i].proto);
    ptr = get_next(ptr,connections[i].recv_q);
    ptr = get_next(ptr,connections[i].send_q);
    ptr = get_next(ptr,connections[i].local_ip);
    ptr = get_next(ptr,connections[i].foreign_ip);
    ptr = get_next(ptr,connections[i].state);
    ptr = get_next(ptr,connections[i].rhiwat);
    ptr = get_next(ptr,connections[i].shiwat);
    ptr = get_next(ptr,connections[i].pid);
    ptr = get_next(ptr,connections[i].epid);

    // find the local port in string (after the dot)
    char *port;
    port = rindex(connections[i].local_ip,0x2e);
    if(port != NULL) {
      port[0] = 0;
      ++port;
      strncpy(connections[i].local_port,port,6);
    }

    // find the remote port in string (after the dot)
    port = rindex(connections[i].foreign_ip,0x2e);
      if(port != NULL) {
      port[0] = 0;
      ++port;
      strncpy(connections[i].foreign_port,port,6);
    }

    // create the commandline string for ps
    char *cmd;
    size = sizeof(char)*strlen(PS_PROC_COMM)+sizeof(char)*strlen(connections[i].pid)+1;
    cmd = malloc(size);
    if(cmd == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    if(snprintf(cmd,size,PS_PROC_COMM,connections[i].pid) < 0) {
      perror("snprintf");
      exit(EXIT_FAILURE);
    }

    // execute ps and store the output to file
    if(system(cmd) == -1) {
      perror(cmd);
      exit(EXIT_FAILURE);
    }
    free(cmd);

    // get the file size for ps output
    if(lstat(FILE_PS,&st_buf) == -1) {
      perror(FILE_PS);
      exit(EXIT_FAILURE);
    }
    size = (size_t)st_buf.st_size;

    // allocate the buffer
    char *ps;
    ps = malloc(size);
    if(ps == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    // read the output from file
    fd = open(FILE_PS,O_RDONLY);
    if(fd == -1) {
      perror(FILE_PS);
      exit(EXIT_FAILURE);
    }
    size_t b_read;
    b_read = read(fd,ps,size);
    if(b_read == -1) {
      perror(FILE_PS);
      exit(EXIT_FAILURE);
    }
    if(close(fd) == -1) {
      perror(FILE_PS);
      exit(EXIT_FAILURE);
    }

    // NOTE: a race condition may exists here
    // file is 4 bytes if pid does not exist
    if(b_read > 5) {

      // NOTE: some processes has no path cointaining slash
      // find last slash
      char *slash;
      slash = rindex(ps,0x2f);

      if(slash != NULL) {

        // skip slash
        char *exe;
        exe = &slash[0];
        ++exe;

        // change the newline to string termination
        char *end = index(exe,0x0a);
        end[0] = 0;

        if(strncpy(connections[i].executable,exe,1024) == NULL) {
          perror("strncpy");
          exit(EXIT_FAILURE);
        }

      } else {

        // find newline
        char *newline;
        newline = index(ps,0x0a);

        if(newline != NULL) {

          // skip newline
          char *exe;
          exe = &newline[0];
          ++exe;

          // change the newline to string termination
          char *end = index(exe,0x0a);
          end[0] = 0;

          if(strncpy(connections[i].executable,exe,1024) == NULL) {
            perror("strncpy");
            exit(EXIT_FAILURE);
          }
        }
      }
    }
    free(ps);

    // lookup full hostname
    struct sockaddr_in sa;
    socklen_t sock_len;
    char hbuf[NI_MAXHOST];
    memset(&sa,0,sizeof(struct sockaddr_in));

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(connections[i].foreign_ip);
    sock_len = sizeof(struct sockaddr_in);

    if(getnameinfo((struct sockaddr *) &sa,sock_len,hbuf,sizeof(hbuf),NULL,0,NI_NAMEREQD) == 0) {
      if(strncpy(connections[i].hostname,hbuf,256) == NULL) {
        perror("strncpy");
        exit(EXIT_FAILURE);
      }
    }

    // lookup as
    char *as = get_mmdb_value(MMDB_ASN_DB,connections[i].foreign_ip,MMDB_ASN);

    if(as) {
      if(strncpy(connections[i].as,as,64) == NULL) {
        perror(as);
        exit(EXIT_FAILURE);
      }
      free(as);
    }

    // lookup country
    char *country = get_mmdb_value(MMDB_CITY_DB,connections[i].foreign_ip,MMDB_COUNTRY);

    if(country) {
      if(strncpy(connections[i].country,country,64) == NULL) {
        perror(country);
        exit(EXIT_FAILURE);
      }
      free(country);
    }

    // location latitude
    char *lat = get_mmdb_value(MMDB_CITY_DB,connections[i].foreign_ip,MMDB_LATITUDE);

    if(lat) {
      if(strncpy(connections[i].latitude,lat,32) == NULL) {
        perror(lat);
        exit(EXIT_FAILURE);
      }
      free(lat);
    }

    // location longitude
    char *lon = get_mmdb_value(MMDB_CITY_DB,connections[i].foreign_ip,MMDB_LONGITUDE);

    if(lon) {
      if(strncpy(connections[i].longitude,lon,32) == NULL) {
        perror(lon);
        exit(EXIT_FAILURE);
      }
      free(lon);
    }

    // plot default value
    connections[i].plot = 0;

    if(strlen(connections[i].latitude) > 0 && strlen(connections[i].longitude) > 0) {
      connections[i].plot = 1;
    }

    // magnitude default value
    float magnitude = MAGNITUDE_VALUE_PER_LOC;

    // calculate magnitude
    for(int c=0; c<i; c++) {
      if(strncmp(connections[i].latitude,connections[c].latitude,32) == 0 &&
        strncmp(connections[i].longitude,connections[c].longitude,32) == 0) {
          magnitude += MAGNITUDE_VALUE_PER_LOC;
          connections[c].plot = 0;
      }
    }

    // build string value from float value
    char magnitude_str[32];
    memset(&magnitude_str,0,32);

    if(snprintf(magnitude_str,32,"%f",magnitude) < 0) {
      perror("snprintf");
      exit(EXIT_FAILURE);
    }
    if(strncpy(connections[i].magnitude,magnitude_str,32) == NULL) {
      perror("magnitude");
      exit(EXIT_FAILURE);
    }

    // print values
    printf("%10s ", connections[i].pid);
    printf("%18s ", connections[i].executable);
    printf("%18s ", connections[i].foreign_ip);
    printf("%52s ", connections[i].hostname);
    printf("%40s ", connections[i].as);
    printf("%24s ", connections[i].country);
    //printf("%8s ",  connections[i].latitude);
    //printf("%8s ",  connections[i].longitude);
    //printf("%8s ",  connections[i].magnitude);
    printf("\n");

  }

  free(buf);

  // create json file
  fd = open(FILE_JSON,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if(fd == -1) {
    perror(FILE_JSON);
    exit(EXIT_FAILURE);
  }

  write(fd,"[[\x22geo-netstat\x22,[",17);

  // add comma false
  int comma = 0;

  for(int i=0; i<num_lines; i++) {

    if(connections[i].plot == 1) {
      if(comma) write(fd,",",1);
      write(fd,connections[i].latitude,strlen(connections[i].latitude));
      write(fd,",",1);
      write(fd,connections[i].longitude,strlen(connections[i].longitude));
      write(fd,",",1);
      write(fd,connections[i].magnitude,strlen(connections[i].magnitude));
      comma = 1;
    }
  }

  write(fd,"]]]",3);
  close(fd);
  exit(EXIT_SUCCESS);
}
char *get_next(char *src, char *dest) {
        char *end;
        size_t len;
        end = index(src,0x20);
        if(end==NULL) end = index(src,0x00);
        len = end-src;
        strncpy(dest,src,len);
        src += len;
        while(src[0]==0x20) ++src;
        return src;
}
char *get_mmdb_value(char *database, char *ip_address, int query){
    MMDB_s mmdb;
    int status;
    status = MMDB_open(database, MMDB_MODE_MMAP, &mmdb);

    if (MMDB_SUCCESS != status) {
        fprintf(stderr, "\n  Can't open %s - %s\n", database, MMDB_strerror(status));

        if (MMDB_IO_ERROR == status) {
            fprintf(stderr, "    IO error: %s\n", strerror(errno));
        }
        return(NULL);
    }

    int gai_error;
    int mmdb_error;
    MMDB_lookup_result_s result;
    result = MMDB_lookup_string(&mmdb, ip_address, &gai_error, &mmdb_error);

    if (0 != gai_error) {
        fprintf(stderr, "\n  Error from getaddrinfo for %s - %s\n\n", ip_address, gai_strerror(gai_error));
        return(NULL);
    }

    if (MMDB_SUCCESS != mmdb_error) {
        fprintf(stderr, "\n  Got an error from libmaxminddb: %s\n\n", MMDB_strerror(mmdb_error));
        return(NULL);
    }

    MMDB_entry_data_list_s *entry_data_list = NULL;

    if (result.found_entry) {
        int status = MMDB_get_entry_data_list(&result.entry, &entry_data_list);

        if (MMDB_SUCCESS != status) {
            fprintf(stderr, "MMDB_get_entry_data_list: Got an error looking up the entry data - %s\n", MMDB_strerror(status));
            goto end;
        }

        if (NULL != entry_data_list) {

            MMDB_entry_data_s entry_data;
            memset(&entry_data,0,sizeof(MMDB_entry_data_s));
            int status;
            char *dst;
            dst = NULL;

            if(query == MMDB_DUMP_ALL) {
              MMDB_dump_entry_data_list(stdout, entry_data_list, 2);
              goto end;
            }
            if(query == MMDB_ASN) {
              status = MMDB_get_value(&result.entry, &entry_data, "autonomous_system_organization", NULL);
              if (MMDB_SUCCESS != status) {
                  //fprintf(stderr, "ASN: Got an error looking up the entry data - %s\n", MMDB_strerror(status));
                  goto end;
              }
              size_t n = entry_data.data_size;
              dst = malloc(n+1);
              if(dst == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
              }
              memset(dst,0,n+1);
              memcpy(dst,entry_data.utf8_string,n);
            }
            if(query == MMDB_COUNTRY) {
              status = MMDB_get_value(&result.entry, &entry_data, "country", "names", "en", NULL);
              if (MMDB_SUCCESS != status) {
                  //fprintf(stderr, "COUNTRY: Got an error looking up the entry data - %s\n", MMDB_strerror(status));
                  goto end;
              }
              size_t n = entry_data.data_size;
              dst = malloc(n+1);
              if(dst == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
              }
              memset(dst,0,n+1);
              memcpy(dst,entry_data.utf8_string,n);
            }
            if(query == MMDB_LATITUDE) {
              status = MMDB_get_value(&result.entry, &entry_data, "location", "latitude", NULL);
              if (MMDB_SUCCESS != status) {
                  //fprintf(stderr, "LATITUDE: Got an error looking up the entry data - %s\n", MMDB_strerror(status));
                  goto end;
              }
              size_t n = (size_t) sizeof(entry_data.double_value);
              dst = malloc(n+1);
              if(dst == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
              }
              memset(dst,0,n+1);
              if(snprintf(dst,n,"%lf",entry_data.double_value) < 0)
              {
                perror("latitude");
                exit(EXIT_FAILURE);
              }
            }
            if(query == MMDB_LONGITUDE) {
              status = MMDB_get_value(&result.entry, &entry_data, "location", "longitude", NULL);
              if (MMDB_SUCCESS != status) {
                  //fprintf(stderr, "LONGITUDE: Got an error looking up the entry data - %s\n", MMDB_strerror(status));
                  goto end;
              }
              size_t n = (size_t) sizeof(entry_data.double_value);
              dst = malloc(n+1);
              if(dst == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
              }
              memset(dst,0,n+1);
              if(snprintf(dst,n,"%lf",entry_data.double_value) < 0) {
                perror("longitude");
                exit(EXIT_FAILURE);
              }
            }
            return(dst);
        }
    } else {
        //fprintf(stderr, "\n  No entry for this IP address (%s) was found\n\n", ip_address);
        return(NULL);
    }

    end:
        MMDB_free_entry_data_list(entry_data_list);
        MMDB_close(&mmdb);
        return(NULL);
}
