#!/bin/sh

NAME=wheel
PIDFILE=/var/run/$NAME.pid
CONFIG=/etc/$NAME/$NAME.conf
DAEMON=/usr/bin/$NAME
TEMPFILE=/tmp/$NAME

PATH=/sbin:/bin:/usr/sbin:/usr/bin

start () {
	echo -n "Starting $NAME \n"
	DAEMON_OPT=$CONFIG
	if [ $# != 0 ] 
	then 
		DAEMON_OPT="${DAEMON_OPT} ${1}"
		echo "${1}" > $TEMPFILE
	fi
	start-stop-daemon --name $NAME --background --quiet --start --make-pidfile --pidfile $PIDFILE --exec $DAEMON -- $DAEMON_OPT
}

stop () {
	echo -n "Stopping $NAME \n"
	start-stop-daemon --stop --quiet --pidfile $PIDFILE --signal TERM --oknodo
	rm -f $PIDFILE
	rm -f $TEMPFILE
}

case $1 in
	start)
		if [ $# = 0 ]
		then
			start
		else
			start $2
		fi
	;;

	stop)
		stop
	;;	

	restart)
		stop
		if [ $# = 0 ]
		then
			start
		else
			start $2
		fi
	;;
	
	status)
		if [ -e $TEMPFILE ]; then
			STAT=$(cat $TEMPFILE)
			echo "${STAT}"
		fi
	;;
	
	*)
		echo "Usage: $NAME {start|stop|restart|status}"
	exit 1

esac

exit 0
