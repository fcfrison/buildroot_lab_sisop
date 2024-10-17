modprobe sstf-iosched
echo sstf > /sys/block/sdb/queue/scheduler
cat /sys/block/sdb/queue/scheduler
sector_read