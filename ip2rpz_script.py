#!/usr/bin/env python3
import sys
import os
import time
from datetime import datetime

ORIGIN = "rewrite.invalid."
ACTION = "IN CNAME ."

def main():
    if len(sys.argv) < 2:
        return -1
    
    serial = datetime.now().strftime("%Y%m%d%H")
    print(f"$ORIGIN {ORIGIN}")
    print("$TTL 60")
    print("@                       SOA  rewrite.invalid. nobody.invalid. (")
    print(f"                        {serial} ; serial")
    print("                        3600       ; refresh (1 hour)")
    print("                        3600       ; retry (1 hour)")
    print("                        86400      ; expire (1 day)")
    print("                        60         ; minimum (1 minute)")
    print("                        )")
    print("                        IN NS localhost.")
    
    for file_name in sys.argv[1:]:
        with open(file_name, "r") as f:
            is6 = False
            for line in f:
                line = line.strip()
                if not line or line.startswith("#") or line.startswith(";"):
                    continue
                
                parts = line.split("/")
                ip_part = parts[0]
                prefix = 128 if ":" in ip_part else 32
                if len(parts) > 1:
                    prefix = int(parts[1])

                if ":" in ip_part:
                    is6 = True
                    ip6 = ip_part
                    rbuf = ""
                    blank = False
                    for i in range(len(ip6)-1, -1, -1):
                        if ip6[i] == ":":
                            ip6 = ip6[:i]
                            if not ip6:
                                if blank:
                                    continue
                                rbuf += "zz"
                                blank = True
                            else:
                                rbuf += ip6[i+1:]
                                blank = False
                            rbuf += "."
                    rbuf += ip6
                    rr = f"{prefix}.{rbuf}.rpz-ip {ACTION}"
                else:
                    ip = ip_part.split(".")
                    rr = f"{prefix}.{int(ip[3])}.{int(ip[2])}.{int(ip[1])}.{int(ip[0])}.rpz-ip {ACTION}"
                
                print(rr)

if __name__ == "__main__":
    main()
