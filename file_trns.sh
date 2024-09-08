USERNAME=root
HOST="${USERNAME}@192.168.1.2"
PATH=/${USERNAME}
FILES=./build/nocturne
OPENCL=./src/image_functions.cl

/usr/bin/scp $FILES $HOST:$PATH
/usr/bin/scp $OPENCL $HOST:$PATH