#!/bin/bash

# CRTDIR-pwd
CRTDIR=$(pwd)
echo $CRTDIR

# asnode/bin
optimized_bin="/usr/local/asnode/sbin/optimized.sh"
$optimized_bin
echo $optimized_bin " started"

# nginx-cache
mkdir -p /data/nginx/cache

# nginx-sock
rm -rf /var/run/nginx
mkdir -p /var/run/nginx

# openresty/logs
mkdir -p /usr/local/openresty/nginx/conf/domain/
mkdir -p /usr/local/openresty/nginx/conf/stream/
mkdir -p /usr/local/openresty/nginx/conf/lua/
mkdir -p /usr/local/openresty/nginx/logs/domain/
mkdir -p /usr/local/openresty/nginx/logs/stream/

# nginx/bin
nginx_bin="/usr/local/openresty/nginx/sbin/nginx"
$nginx_bin
echo $nginx_bin " started"

# asnode/bin
asnode_bin="/usr/local/asnode/sbin/Asnode"
$asnode_bin
echo $asnode_bin " started"
