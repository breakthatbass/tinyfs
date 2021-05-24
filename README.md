# tiny-file-sharer

share files between seperate systems in a LAN through the terminal

## usage:
to send:
```
send -i <ip-of-destination> -p <port>
```
to receive:
```
recv -p <port>
```
`send` boots up the server, connects the client and waits for the the `recv` client to connect.  
when it does, `send` will send send the file to the `recv` client.

**getting ip addresses of systems**  
on linux
```
hostname -I | awk '{print $1}'
```
on macos you can do two things. the command line is a bit tougher because `hostname` on macos does not have the `-I` option.
`ifconfig` will return a giant mess, it's in there if you can find it.  
i was able to parse it on my system with this:
```
ifconfig | grep "\<inet\>" | awk '{print $2}' | tail -n 1
```
However, I don't know if `ifconfig` will display things the same on all systems.  
Fortunately, the IP easily acessible in the network settings.