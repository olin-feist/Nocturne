USERNAME=root
HOST="${USERNAME}@192.168.1.2"
PATH=/${USERNAME}
FILES=./build/nocturne

/usr/bin/scp $FILES $HOST:$PATH