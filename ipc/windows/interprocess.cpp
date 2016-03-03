#include "interprocess.h"

interprocess::interprocess(std::wstring _filename): 
    filename(_filename.begin(), _filename.end()), pipe_handle(NULL) {
    std::stringstream ss;
    ss<<TEXT("\\\\.\\pipe\\")<<filename;
    filename = ss.str();
 
}


void interprocess::server_create()
{
    pipe_handle = CreateNamedPipe(filename.c_str(), PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT,
                                  1, BUFSIZE*sizeof(TCHAR), 
                                  BUFSIZE*sizeof(TCHAR), PIPE_TIMEOUT, NULL);

    if (pipe_handle == INVALID_HANDLE_VALUE) {
        if (GetLastError() == 231)
            throw ipc::server_process_connected();
        throw ipc::fifo_create_error();
    }


}

void interprocess::client_create()
{

    if (pipe_handle == NULL) {
        pipe_handle = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (pipe_handle == INVALID_HANDLE_VALUE) {
            throw ipc::fifo_open_error();
        }
    }
}


void interprocess::send_msg(std::string msg) 
{
    client_create();
    msg = on_msg_send(msg);
    int size = msg.size();
    std::lock_guard<std::mutex> lock(read_mutex);
    DWORD kur;
    WriteFile(pipe_handle, (const char*)(&size), sizeof(size), &kur, NULL);
    WriteFile(pipe_handle, msg.c_str(), size, &kur, NULL);

}

void interprocess::listen()
{
    server_create(); 

    char* buff;
    int size;
    while (true) {
       bool connected = ConnectNamedPipe(pipe_handle, NULL);
        if (!connected) {
            int error = GetLastError();
            if (error != ERROR_PIPE_CONNECTED) {
                cout<<error<<endl;
                throw ipc::fifo_open_error();
            }
            
        }
        DWORD num_bytes_read;
        ReadFile(pipe_handle,(char*)(&size), sizeof(size), &num_bytes_read,NULL);
        if (num_bytes_read == sizeof(size)) {
            buff = new char[size+1];
            ReadFile(pipe_handle, buff, size, &num_bytes_read, NULL);

            if (num_bytes_read != size) {
                delete[] buff;
                throw ipc::wrong_msg_size();
            }
            buff[size] = '\0';
            last_msg = std::string(buff);
            cout<<"received: "<<last_msg<<endl;
            delete[] buff;

            th = std::thread(&interprocess::on_msg_receive, this, std::ref(last_msg));
            th.detach();
            if(last_msg == "quit")
                break;
        }
    }

}

interprocess::~interprocess()
{
    CloseHandle(pipe_handle);
}
