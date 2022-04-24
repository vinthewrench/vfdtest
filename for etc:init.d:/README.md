install

sudo nano /etc/init.d/vfd

---

```bash
 #!/bin/bash
# /etc/init.d/fan

### BEGIN INIT INFO

# Provides:vfd
# Required-Start:$remote_fs $syslog
# Required-Stop:$remote_fs $syslog
# Default-Start:2 3 4 5
# Default-Stop:0 1 6
# Short-Description: vfd
# Description: vfd auto start after boot
### END INIT INFO

case "$1" in
    start)
        echo "Starting vfd"
        cd /home/vinthewrench/vfd/bin
        nohup /home/vinthewrench/vfd/bin/vfd &
         ;;
    stop)
        echo "Stopping vfd"
     	 killall -9 vfd
         ;;
    *)
        echo "Usage: service vfd start|stop"
        exit 1
        ;;
esac
exit 0

```

sudo chmod +x /etc/init.d/vfd
sudo update-rc.d vfd defaults

To remove :

sudo update-rc.d  -f vfd remove

sudo service vfd start # start the service
sudo service vfd stop # stop the service
sudo service vfd restart # restart the service
sudo service vfd status # check service status