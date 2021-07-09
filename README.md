# tinyfs
**Tiny File Sharer**  
Share files between seperate systems in a LAN through the terminal.  

`tinyfs` is only about 300 lines of code. The sender can send a file of any size and, on success, the reciever will receive that file with the same file name in their current directory.  

It works with files of any type.

It works on MacOS and Linux.

## Installation
```
git clone https://github.com/breakthatbass/tinyfs.git
cd tinyfs
make install
```

## Usage:
to send:
```
send -h <hostname/ipaddress> -p <port> -f <file>
```
`hostname` and `file` are required. Note: sender must send before reciever tries to recieve the file.

to receive:
```
recv -h <hostname/ipaddress> -p <port>
```
`hostname` is required.  

`port` defaults to `3490` for both `send` and `recv`. If the sender changes the port, the receiver must supply the same port.
#
**getting ip addresses of systems**  

`arp -a` will list all the ip address on the network, however, it's pretty hard to tell what is what. It's easier to get the ip address on the systems themselves.

**on Linux**
```
hostname -I | awk '{print $1}'
```
 
**on MacOS**
```
ifconfig    # returns a giant mess but it's in there.
```
I was able to parse out the IP on my own system with this:
```
ifconfig | grep "\<inet\>" | awk '{print $2}' | tail -n 1
```
However, I don't know if `ifconfig` will display things the same on all systems.  

If that doesn't work for MacOS, you can always get it in your network settings.

## Is it secure?
I doubt it.

## Contributing
If you find a bug it would be great if you open an issue. And if you want to fix it or contribute in any way, pull requests are appreciated.