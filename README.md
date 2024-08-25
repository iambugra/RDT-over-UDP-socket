# Reliable Data Transfer Over SOCK_DGRAM

*This project hosts the code for a homework assignment of CENG 435 - Data Communications and Networking course in Computer Engineering at Middle East Technical University.*

## Case
The task is to implement a private chat program over a network where the packets might be reordered, dropped, delayed or garbled. The payload size of the packets sent over the network is also limited to 8 bytes. Finally, the implementation should not use SOCK_STREAM, kernel's TCP implementation, but use SOCK_DGRAM.

## Implementation
C++ is used to implement this task. The code also utilizes `pthread` library.

To ensure reliable transer on the unreliable channel, checksums are implemented on every packet that is being sent. ACK packets are also used to make sure the other side got the packet right, and a timer mechanism is implemented to trigger a timeout for lost packets. In order to maximize the channel utilization, Selective Repeat protocol is used.

## Run
To use it locally, run:
```
./troll -C 127.0.0.1 -S 127.0.0.1 -a <server-port> -b <client-port> <troll-port>
./server <server-port>
./client 127.0.0.1 <troll-port> <client-port>
```
