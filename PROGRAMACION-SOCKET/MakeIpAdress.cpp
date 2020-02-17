#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> // para sockaddr_in
#include <string>
#include <cerrno>		// para errno
#include <cstring>		// para std::strerror()

sockaddr_in make_ip_address(const std::string& ip_address, int port)
{
    // Dirección del socket local
    sockaddr_in local_address {};	// Así se inicializa a 0, como se recomienda
    local_address.sin_family = AF_INET;	// Pues el socket es de dominio AF_INET
    if(ip_address == "")
        local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        local_address.sin_addr.s_addr = inet_addr(ip_address.c_str());
    local_address.sin_port = htons(port);
    inet_aton("X.X.X.X", &local_address.sin_addr);

    return local_address;
}
