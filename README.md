# tinyfs
**Tiny File Sharer**  
Share files between seperate systems in a LAN through the terminal.  

`tinyfs` is only about 300 lines of code. The sender can send a file of any size and, on success, the reciever will receive that file with the same file name in their current directory.

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

## How it works:
It's a pretty basic client/server program. `send` is a server that reads in the file and passes the file name and then the file contents through a socket to the `recv` which actas as a client.

`send` can start and it'll wait for a client connection. `revc` connects, creates a file based on the file name sent through, then writes the contents that are sent through to the file.

## Why?
I often will start the day working on my laptop somewhere comfortable with coffee. A typical situation is I code in order to test soemthing out but don't want to push any of the code. At some point I move to my desktop and may want to continue working with it or copy and paste parts of it into whatever larger program I'm working on.

I can add it dropbox or email it to myself but I always want less steps. So `tinyfs` is an attempt to automate that. I'll use this program. I don't know that anyone else will.