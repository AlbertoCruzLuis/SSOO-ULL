#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>  // para inet_ntoa()   
#include <unistd.h>     // para close()
#include <system_error>
#include <set>
#include <tuple>
#include "MakeIpAdress.cpp"
#include "Message.cpp"

class Socket
{
    public: 
        Socket(const sockaddr_in&);
        ~Socket();

        //Getters
        int get_fd()
        { return fd_; }

        //Setters
        void set_fd(int fd)
        { fd_ = fd; }

        void send_to(const Message&, const sockaddr_in&);
        void receive_from(Message&, sockaddr_in&);

    private:
        int fd_;
};