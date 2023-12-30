/* compile: cc -s -o ip2res-ip ip2res-ip.c */
/*
  ./ip2res-ip <list file>
  eg:
  ./ip2res drop.txt all.txt banlist.txt
 
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <arpa/inet.h>

#define ORIGIN "rewrite.invalid."
#define ACTION "IN CNAME ."

int main(int argc, char *argv[])
{
  uint8_t ip[4];
  uint8_t ip6[16];
  char buf[1024];
  char rbuf[55];
  char rr[512];
  char serial[200];
  time_t s;
  struct tm *ts;
  char *p, *q;
  int prefix;
  int is6, i, d, blank;
  FILE *f;

  if (argc < 2) return -1;
  s = time(NULL);
  ts = localtime(&s);
  if (ts == NULL) {
    fprintf(stderr, "localtime failed\n");
    return -1;
  }
  if (strftime(serial, sizeof(serial), "%Y%m%d%H", ts) == 0) {
    fprintf(stderr, "strftime returned 0");
    return -1;
  }
  fprintf(stdout,"$ORIGIN %s\n", ORIGIN);
  fprintf(stdout,"$TTL 60\n");
  fprintf(stdout,"@                       SOA  rewrite.invalid. nobody.invalid. (\n");
  fprintf(stdout,"                        %-10s ; serial\n", serial);
  fprintf(stdout,"                        3600       ; refresh (1 hour)\n");
  fprintf(stdout,"                        3600       ; retry (1 hour)\n");
  fprintf(stdout,"                        86400      ; expire (1 day)\n");
  fprintf(stdout,"                        60         ; minimum (1 minute)\n");
  fprintf(stdout,"                        )\n");
  fprintf(stdout,"                        IN NS localhost.\n");
  for (d = 1; d < argc; d++){
    f = fopen(argv[d], "r");
    if (!f) continue;
    is6 = 0;
    while (fgets(buf, sizeof(buf) - 1, f) != NULL) {
      p = buf;
      while(isblank(*p)) p++;
      q = p;
      while(!isspace(*q)) q++;
      *q = 0;
      if (*p == '#') continue;
      if (*p == ';') continue;
      if (strchr(p, ':')) is6 = 1;
      q = strchr(p, '/');
      if (!q || !*q) {
        if (is6) prefix = 128;
        else prefix = 32;
      } else {
        *q = 0;
        q++;
        prefix = atoi(q);
      }
      if (!strlen(buf)) continue;
      if (is6) {
        if (!inet_pton(AF_INET6, p, &ip6)) {
          fprintf(stderr, "invalid ip6 address: %s\n", buf);
          continue;
        }
        if (!inet_ntop(AF_INET6, &ip6, buf, sizeof(buf))) {
          fprintf(stderr, "inet_ntop() failed\n");
          continue;
        }
        strcpy(rbuf, "");
        blank = 0;
        for (i = strlen(buf); i--;) {
          if (buf[i] == ':') {
            buf[i] = 0;
            if (!strlen(&buf[i+1])) {
	    if (blank) continue;
              strcat(rbuf, "zz");
	    blank = 1;
            } else {
              strcat(rbuf, &buf[i+1]);
              blank = 0;
            }
            strcat(rbuf, ".");
          }
        }
        strcat(rbuf, buf);
        snprintf(rr, sizeof(rr), "%d.%s.rpz-ip %s", prefix, rbuf, ACTION)
          < 0 ? abort() : (void)0;
      } else {
        if (!inet_pton(AF_INET, p, &ip)) {
          fprintf(stderr, "invalid ip address: %s\n", buf);
          continue;
        }
        snprintf(rr, sizeof(rr), "%d.%d.%d.%d.%d.rpz-ip %s",
          prefix, (int)ip[3], (int)ip[2], (int)ip[1], (int)ip[0],
          ACTION) < 0 ? abort() : (void)0;
      }
      fprintf(stdout, "%s\n", rr);
      is6 = 0;
    }
    fclose(f);
  }
}
