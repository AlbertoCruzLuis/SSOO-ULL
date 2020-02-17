#include "Socket.hpp"

//Constructor
Socket::Socket(const sockaddr_in& address)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    set_fd(fd);

    if (get_fd() < 0) 
    {
        throw std::system_error(errno,std::system_category(),"no se pudo crear el socket\n");
        //return 3;	// Error. Terminar con un valor diferente y > 0
    }
    std::cout << "socket Creado" << std::endl;

    int result = bind(get_fd(), reinterpret_cast<const sockaddr*>(&address),
                sizeof(address));

    if (result < 0) 
    {
        //std::cerr << "falló bind: " << z << '\n';
        //return 5;	// Error. Terminar con un valor diferente y > 0
    }
}

//Destructor
Socket::~Socket()
{
    close(get_fd());
    std::cout << "fd cerrrado correctamente\n";
}

//Enviar Mensaje
void Socket::send_to(const Message& message, const sockaddr_in& address)
{
    // Enviar un mensaje al socket remoto

    int rsesult = sendto(get_fd(), &message, sizeof(message), 0,
	reinterpret_cast<const sockaddr*>(&address),
    sizeof(address));

    if (rsesult < 0) 
    {
        std::cerr << "falló sendto: " << std::strerror(errno) << '\n';
        //return 6;	// Error. Terminar con un valor diferente y > 0
    }
}

//Recibir Mensaje
void Socket::receive_from(Message& message, sockaddr_in& address)
{
    socklen_t src_len = sizeof(address);

    // Recibir un mensaje del socket remoto
    
    int rresult = recvfrom(get_fd(), &message, sizeof(message), 0,
        reinterpret_cast<sockaddr*>(&address), &src_len);
    if (rresult < 0)
    {
        std::cerr << "falló recvfrom: " << std::strerror(errno) << '\n';
        //return 8;
        //exit(8);
    }
}

