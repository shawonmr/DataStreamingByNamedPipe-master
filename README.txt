This is a server / client application through a NamedPipe. The server and the client are two separate applications.
The applications are built on microsoft visual studio 2017 in C++. 

The server includes a class (e.g. NamedPipeClientData in Server.h) that stores information (e.g. strings, numbers etc.) from the client. 
Both the server and the client keeps a maximum size of 512 number of character buffer for information processing. Both the server and the client 
can handle sync and async calls. A separate thread is created for each of the client's call. Next, each thread recieves information
from the client and then provides an object of a class that stores the corresponding information from the client. Finally,
the thread stores that object in a vector of that class type. Multiple threads may try to store at the same time in the vector 
causing the race condition. As a result, a mutex block has been added when the vector assignment is done.

The client application can send strings and numbers. In this example, a string followed by a number (e.g. between 1 and 999) forms a 
complete message from the client that is stored at the server end. As a result, this number can be used as the client's id and searched 
through the vector to retrieve its corresponding message. The client can send data either sync or async mode. An input prompt provides 
that option at the client end and then the dwMode is change from PIPE_READMODE_MESSAGE to PIPE_NOWAIT
or vice versa depending on the input.When a client leaves the communication, the server prints out the clients that called 
the server and the corresponding thread for the client is also diminished. 

Please run the server application first and then the client. The server application can be closed while clicking the cross button on the
prompt.
