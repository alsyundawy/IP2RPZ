#!/bin/bash

# Function to convert IPv4 address to reverse DNS format
ipv4_to_reverse_dns() {
    local ip=$1
    local prefix=$2
    local action=$3
    local octets
    IFS='.' read -r -a octets <<< "$ip"
    local reverse_dns="${prefix}.${octets[3]}.${octets[2]}.${octets[1]}.${octets[0]}.rpz-ip $action"
    echo "$reverse_dns"
}

# Function to convert IPv6 address to reverse DNS format
ipv6_to_reverse_dns() {
    local ip=$1
    local prefix=$2
    local action=$3
    local ip6
    IFS=':' read -r -a ip6 <<< "$ip"
    local rbuf=""
    local blank=0
    for ((i=${#ip6[@]}-1; i>=0; i--)); do
        if [[ "${ip6[i]}" == "" ]]; then
            if (( blank == 1 )); then
                continue
            fi
            rbuf+="zz."
            blank=1
        else
            rbuf+="${ip6[i]}." 
            blank=0
        fi
    done
    local reverse_dns="${prefix}.${rbuf%.}.rpz-ip $action"
    echo "$reverse_dns"
}

# Main function
main() {
    ORIGIN="rewrite.invalid."
    ACTION="IN CNAME ."
    serial=$(date +"%Y%m%d%H")
    
    echo "\$ORIGIN $ORIGIN"
    echo "\$TTL 60"
    echo "@                       SOA  rewrite.invalid. nobody.invalid. ("
    echo "                        $serial ; serial"
    echo "                        3600       ; refresh (1 hour)"
    echo "                        3600       ; retry (1 hour)"
    echo "                        86400      ; expire (1 day)"
    echo "                        60         ; minimum (1 minute)"
    echo "                        )"
    echo "                        IN NS localhost."
    
    for file in "${@:1}"; do
        if [[ ! -f "$file" ]]; then
            continue
        fi
        while IFS= read -r line; do
            line="${line#"${line%%[![:space:]]*}"}"   # Trim leading whitespace
            if [[ "${line:0:1}" == "#" || "${line:0:1}" == ";" ]]; then
                continue
            fi
            if [[ $line == *":"* ]]; then
                is_ipv6=1
            else
                is_ipv6=0
            fi
            line_split=($line)
            ip="${line_split[0]}"
            if [[ ${line_split[1]+isset} ]]; then
                prefix="${line_split[1]}"
            else
                if (( is_ipv6 == 1 )); then
                    prefix=128
                else
                    prefix=32
                fi
            fi
            if (( is_ipv6 == 1 )); then
                if ! ip6=$(printf "%s" "$ip" | xargs -I{} bash -c 'echo {} | grep -P "([0-9a-fA-F]{1,4}:){7}([0-9a-fA-F]{1,4})"'); then
                    echo "invalid ip6 address: $ip" >&2
                    continue
                fi
                reverse_dns=$(ipv6_to_reverse_dns "$ip" "$prefix" "$ACTION")
            else
                if ! ip4=$(printf "%s" "$ip" | xargs -I{} bash -c 'echo {} | grep -E "([0-9]{1,3}\.){3}[0-9]{1,3}"'); then
                    echo "invalid ip address: $ip" >&2
                    continue
                fi
                reverse_dns=$(ipv4_to_reverse_dns "$ip" "$prefix" "$ACTION")
            fi
            echo "$reverse_dns"
        done < "$file"
    done
}

# Run main function with command line arguments
main "$@"
