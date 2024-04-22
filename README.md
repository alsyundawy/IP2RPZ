compile: cc -s -o ip2res-ip ip2res-ip.c

  ./ip2res-ip <list file>
  eg:
  ./ip2res drop.txt all.txt banlist.txt


./ip2res-ip all.txt banlist.txt drop.txt dropv6.txt bfblocker.txt firehol_level1.txt 2>/dev/null



or 

python3 ip2rpz_script.py ll.txt banlist.txt drop.txt dropv6.txt bfblocker.txt firehol_level1.txt 2>/dev/null

or

bash ip2rpz_script.py ll.txt banlist.txt drop.txt dropv6.txt bfblocker.txt firehol_level1.txt 2>/dev/null
