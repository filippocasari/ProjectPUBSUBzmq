# Project ZMQ PUB-SUB architecture 

Filippo Casari's bechelor thesis project


## _Pub Sub ZeroMQ with Synchronization_


## Components

- Linked-blocking queue
- Publisher and Subscriber
- Zactors (zeromq threads)
- python script to generate new json files
- python script to analize outcoming data

> Publisher: produces strings containing the timestamp and publishes them. It can decide which topic it want to publish on. 
> Subscriber: It subscribes itself to one (or more) topic during the creation of the socket.

For both publisher and subscribers it was used the library called "json-c" which allowed me to read the json configuration files. 
## Publisher
  
It consists on : 
                    _- reading and setting all parameters of the publisher (e.g. ip, port, etc.)_
                    _- creating a new pub socket based on these settings_
                    _- starting a sync loop to get all subs ready for the communication_
                    _-building new messages containing timestamps and sending them through a loop until the number of messages is reached. You can choose a timestamp as milliseconds or nanoseconds_
                    
It is also present a method to get automatically the ip address of the publisher. 

_Note:_ **before** starting sending messages (after the sync) I had to "stop" the program for 4 seconds (reasonably enough) (with the method "zsleep") because otherwise subs are not be able to receive all the messages.

## Subscriber

The subscriber takes 3 arguments: 
        - path of the json config file
        - path of the csv file which it will write the rusults on
        - verbose option
At the beginning it reads json file to set parameters. Then, it starts the sync service (consisting on a request and receive sockets). After these steps, it receives messages from a selected topic. 
The header file SUB.h contains all the main functions of the subscriber. The method subscriber consists on :
- creating a new blocking queue
- starting a loop to receive messages and menage them through a method called "payload managing". It pushes the body of the message (timestamp of sending), the timestamp of receiving, the name of the metric and the number of the message into a new instance of the class item. 
- this loop ends when the all messages are received or it reaches a Timeout or there is a kind of interrupt
- then, having a queue of Items, it tries to write them on a csv file and store them. 
