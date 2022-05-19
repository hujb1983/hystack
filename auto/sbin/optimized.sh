#!/bin/bash

echo "" >> /etc/sysctl.conf
echo "# Asnode configuer for optimized." >> /etc/sysctl.conf
echo "net.core.somaxconn=10000" >> /etc/sysctl.conf
echo "net.ipv4.tcp_max_syn_backlog=250000" >> /etc/sysctl.conf
echo "net.ipv4.tcp_slow_start_after_idle=1" >> /etc/sysctl.conf
echo "net.core.wmem_default=327680" >> /etc/sysctl.conf
echo "net.core.wmem_max=67108864" >> /etc/sysctl.conf
echo "net.core.rmem_default=327680" >> /etc/sysctl.conf
echo "net.core.rmem_max=67108864" >> /etc/sysctl.conf
echo "net.ipv4.tcp_wmem=4096 87380 67108864" >> /etc/sysctl.conf
echo "net.ipv4.tcp_rmem=4096 87380 67108864" >> /etc/sysctl.conf

# echo "net.ipv4.tcp_loss_init_cwnd=10" >> /etc/sysctl.conf
echo "net.ipv4.ip_local_port_range=1024 65535" >> /etc/sysctl.conf

# echo "net.ipv4.tcp_rto_max=1" >> /etc/sysctl.conf
# echo "net.ipv4.tcp_rto_min=50" >> /etc/sysctl.conf

echo "net.ipv4.tcp_frto=2" >> /etc/sysctl.conf
echo "net.ipv4.tcp_retries1=100" >> /etc/sysctl.conf
echo "net.ipv4.tcp_retries2=120" >> /etc/sysctl.conf
echo "net.ipv4.tcp_syn_retries=10" >> /etc/sysctl.conf
echo "net.ipv4.tcp_synack_retries=15" >> /etc/sysctl.conf
echo "net.ipv4.tcp_orphan_retries=150" >> /etc/sysctl.conf
echo "net.ipv4.tcp_no_metrics_save=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_tw_recycle=0" >> /etc/sysctl.conf
echo "net.ipv4.tcp_tw_reuse=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_fastopen=1" >> /etc/sysctl.conf

# echo "net.ipv4.tcp_init_cwnd=120" >> /etc/sysctl.conf

echo "net.ipv4.tcp_timestamps=0" >> /etc/sysctl.conf
echo "net.ipv4.tcp_fastopen=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_moderate_rcvbuf = 1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_max_tw_buckets=180000" >> /etc/sysctl.conf
echo "net.ipv4.icmp_echo_ignore_broadcasts=1" >> /etc/sysctl.conf
echo "net.ipv4.icmp_ignore_bogus_error_responses=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_sack=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_fack=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_dsack=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_init_cwnd=360" >> /etc/sysctl.conf
echo "net.ipv4.tcp_wmem=4096 1048576 4194304" >> /etc/sysctl.conf
echo "net.ipv4.tcp_timestamps=0" >> /etc/sysctl.conf
echo "net.ipv4.tcp_syncookies=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_tw_reuse=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_tw_recycle = 1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_fastopen = 3" >> /etc/sysctl.conf
