Here should lie the implementation for the ipc we are using
Every platform-specific file _must_ follow the model specified by `interprocess.h`

##Usage
Include `get.h` file and inherit the interprocess class.
Implement:
* `std::string on_msg_send(std::string&)` - how to transform the message before sending it 
* `std::string on_msg_receive(std::string&)` - how to interpret the message hen received

`on_msg_receive` is ran in another thread, so you must take care of synchronization

After that use `send_msg` to send messages to the process which called `listen`

##TODO
*   Make windows implementation
