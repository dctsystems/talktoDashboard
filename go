#Make a tmp/ram filesystem
sudo umount /tmp/LIVE
mkdir /tmp/LIVE
sudo mount -t tmpfs -o size=16M,mode=777 tmpfs /tmp/LIVE

#enable PiCam
sudo modprobe bcm2835-v4l2

export DASHBIN=`pwd`/bin
export DASHLIB=`pwd`/lib

talkto/talkto &
alexaParser/alexaParser &
