//==============================================================
//@Autor: Alberto Cruz Luis
//@Email: alu0101217734@ull.edu.es
//@Fecha: Diciembre 2019
//@Name: Netcp.cpp
//@Version: Practica Programacion-Socket
//==============================================================

#include "Socket.cpp"
#include <thread>
#include <pthread.h>
#include <atomic>
#include <csignal>
#include <signal.h>
#include <vector>

std::atomic<bool> quit(false);  //Para hilos(Modo_Cliente,Modo_Servidor)
std::atomic<bool> quit_Interno(false);  //Para hilos(Ficheros)
std::thread *hiloServer;
bool Salir = false; //Variable para Salir del Programa

//Manual del Programa
void Usage()
{
    std::cout << "\tMANUAL DE USO\n";
    std::cout << "SYNOPSIS\n";
    std::cout << "\t./NetcpSend [Opciones]\n";

    std::cout << "DESCRIPCION\n";
    std::cout << "\tPrograma para Enviar y Recibir Archivos\n";

    std::cout << "OPCIONES\n";
    std::cout << "\t[-s]        Crear Modo Servidor\n";
    std::cout << "\t[-c]        Crear Modo Cliente\n";
}

struct CommandLineArguments
{
    bool show_help = false;
    bool server_mode = false;
    unsigned short conn_port = 0;
    std::string ip = "";
    std::vector<std::string> other_arguments;

    CommandLineArguments(int argc, char* argv[]);
};

CommandLineArguments::CommandLineArguments(int argc, char* argv[])
{
        // Mira getopt(argc, argv, "hsp:01") mas abajo, en el while:
    //
    // 	"hsp:01" indica que nuestro programa acepta las opciones
    // 	"-h", "-s", "-p", "-0" y "-1".
    //
    // 	El "p:" en la cadena indica que la opción "-p" admite un
    // 	argumento de la forma "-p argumento"

    // 	En cada iteración la variable "c" contiene la letra de la
    // 	opción encontrada. Si vale -1, es que ya no hay más
    // 	opciones en argv.
        int c;
        while ( (c = getopt(argc, argv, "hsc:p:01")) != -1)
        {
            // Recuerda que "c" contiene la letra de la opción.
            switch (c) 
            {
                case '0':
                case '1':
                    std::cerr << "opción " << c << std::endl;
                    break;
                case 'h':
                    //Manual del Programa
                    std::cerr << "opción h\n";
                    Usage();
                    show_help = true;
                    break;
                case 's':
                    //Modo Servidor
                    std::cerr << "opción s\n";
                    server_mode = true;
                    break;
                case 'c':
                    //Modo Cliente
                    std::cerr << "opción c\n";
                    ip = optarg;
                    break;
                case 'p':
                    // Esta opción recibe un argumento.
                    // getopt() lo guarda en la variable global "optarg"
                    std::cerr << "opción p con valor " << optarg << std::endl;
                    conn_port = std::atoi(optarg);
                    break;
                case '?':
                    // c == '?' cuando la opción no está en la lista
                    // getopt() se encarga de mostrar el mensaje de error.
                        throw std::invalid_argument("Argumento de línea de comandos " 
                    "desconocido");
                default:
                    // Si "c" vale cualquier otra cosa, algo fue mal con
                    // getopt(). Esto no debería pasar nunca.
                    throw std::runtime_error("Error procesando la línea de "
                    "comandos");
            }
        }

        // Llegados aquí hemos procesado todas las opciones.
        // Es decir, todos los argumentos del tipo -<letra>.
        //
        // Por ejemplo, en el comando "ls -lt -R /bin/sh" son opciones
        // 'l', 't' y 'R'. Mientras que '/bin/sh' es un argumento para el
        // programa pero no es una opción.

        // En este punto la variable global "optind" contiene el índice del
        // argumento de la línea de comandos que no es una opción.
        // Si optind == argc, es que ya no quedan más argumentos en argv
        // para procesar.
        if (optind < argc) 
        {
            std::cerr << "-- empezamos con los argumentos no opción --\n";
            for (; optind < argc; ++optind) 
            {
                std::cerr << "argv[" << optind << "]: " <<
                argv[optind] << '\n';
                other_arguments.push_back(argv[optind]);
            }
        }
}

//Solicitud Hilos por referencia
void request_cancellation(std::thread& thread)
{
    int result = pthread_cancel(thread.native_handle());
    if(result != 0)
    {
        std::cerr << "Error en request_cancellation\n";
    }
}

//Solicitud hilos por punteros
void request_cancellation(std::thread* thread)
{
    int result = pthread_cancel(thread->native_handle());
    if(result != 0)
    {
        std::cerr << "Error en request_cancellation\n";
    }
}

//Controla Imprevistos como Ctrl -C, Shutdown...
void int_signal_handler(int signum)
{
    if(signum == SIGINT || signum == SIGTERM || signum == SIGHUP)
    {
        quit = true;
        std::cout << "Señal interceptada\n";
        request_cancellation(hiloServer);
    }
}

//Hilo Modo Cliente
void Hilo_Cliente(Message& message, sockaddr_in& address, Socket& socket)
{
    pthread_testcancel();
    int Opcion =0;
    std::cout << "Pulsa 1: Salir Programa\n";
    std::cout << "Pulsa 2: Modo Enviar\n";
    std::cin >> Opcion;

    if(Opcion == 2)
    {
        std::string Nom_Fichero;
        std::cout << "Escribe el nombre del fichero a enviar: \n";
        std::cin >> Nom_Fichero;

        //Asignar el nombre del fichero de lectura
        Nom_Fichero.copy(message.nombreArchivo.data(), Nom_Fichero.length(),0);

        //Y una ruta donde almacenar los ficheros (Directorio)
        std::string Fichero_Lectura = "./ClientFile/" + Nom_Fichero;

        int DatosLeidos = message.text.size();

        //Apertura del fichero
        int Archivo = open(Fichero_Lectura.c_str(),O_RDONLY);
        if(Archivo < 0)
        {
            std::cerr << "El fichero " << Nom_Fichero << " no existe\n";
        }else
        {
            std::cout << "INT_ARCHIVO: " << Archivo << std::endl;

            //Asignamos el identificador del archivo
            message.identificador = Archivo;

            //Contador para identificar tipo de mensaje
            int Cont = 0;

            // Enviar un mensaje al socket remoto   
            while(DatosLeidos >= message.text.size()-1)
            {   
                //Leer Mensaje
                DatosLeidos = read(message.identificador,message.text.data(),
                message.text.size() -1);
                std::cout << "DATOS_LEIDOS: " << DatosLeidos << std::endl;
                //Tamaño del mensaje leido
                message.Tam_text = DatosLeidos;
                

                //Identificar Tipo de Mensaje y asignar idenficador del archivo
                if(Cont == 0 && DatosLeidos >= message.text.size()-1)
                    message.tipoMensaje = 1;
                else if(Cont == 0 && DatosLeidos < message.text.size()-1)
                    message.tipoMensaje = 4;
                else if(Cont != 0 && DatosLeidos < message.text.size()-1)
                    message.tipoMensaje = 3;
                else
                    message.tipoMensaje = 2;
                
                //Enviar Mensaje
                socket.send_to(message,address);
                Cont++;
            }
        }
        close(Archivo);
        Salir = false;
    } else if(Opcion == 1)
        Salir = true;
    else
    {
        std::cout << "Opcion incorrecta\n";
        Salir = false;
    }
    quit_Interno = true;
}

//MODO CLIENTE
void Modo_Cliente(std::exception_ptr& eptr, unsigned short puerto, std::string ip)
{
    //Bloquear Señales para este hilo
    sigset_t set;
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGHUP);

    //pthread_testcancel();

    int s;  //Valor de pthread_sigmaskPROGRAMACION (1)

    s = pthread_sigmask(SIG_BLOCK, &set, nullptr);
    if(s != 0)
        std::cout << "Error en pthread_sigmask \n";

    try
    {
        Message message;

        //Preparar la direccion del Socket Remoto
        sockaddr_in remote_address {};
        remote_address = make_ip_address(ip,puerto);

        Socket Cliente(remote_address);
        std::cout << "Cliente\n";

        std::set<std::pair<int, std::thread *>> active_threads;
        std::thread *Hilo;
        

        while(Salir == false)
        {
            Hilo = new std::thread (&Hilo_Cliente, std::ref(message),
            std::ref(remote_address), std::ref(Cliente));
            active_threads.insert(std::make_pair(message.identificador,Hilo));

            while(!quit_Interno);
            quit_Interno = false;

            //equest_cancellation(Hilo);
            for(auto i : active_threads)
            {
                request_cancellation(i.second);
                Hilo->join();
                active_threads.erase(i);
            }
        }
    } catch(const std::exception& e)
    {
        eptr = std::current_exception();
    }
    //Se acabo este hilo
    quit = true;
}

//MODO SERVIDOR
void Modo_Servidor(std::exception_ptr& eptr, unsigned short puerto)
{
    //Bloquear Señales para este hilo
    sigset_t set;
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGHUP);

    int s;  //Valor de pthread_sigmask

    pthread_testcancel();

    int DatosEscritos = 1;

    s = pthread_sigmask(SIG_BLOCK, &set, nullptr);
    if(s != 0)
        std::cout << "Error en pthread_sigmask \n";

    try
    {
        Message message;
        //int DatosEscritos;

        //Crear socket local

        //Asignar Direccion al socket local
        sockaddr_in local_address{};
        local_address = make_ip_address("127.0.0.1",puerto);
        Socket Servidor(local_address);
        std::cout << "Servidor\n";

        //fd = open("prueba.txt",O_WRONLY);

        std::cout<< "Todo Correcto" << std::endl;
        // Recibir un mensaje del socket remoto

        while(!quit)
        {
            Servidor.receive_from(message,local_address);

            //Creacion del Set
            std::set<std::tuple<uint32_t, in_port_t, std::string, int>> receiving_data;
            //Pasar Toda la informacion del Mesage a la Tupla
            std::tuple<uint32_t, in_port_t, std::string, std::string, int, int, int> tupla;
            tupla = std::make_tuple(local_address.sin_addr.s_addr,local_address.sin_port,
            message.nombreArchivo.data(), message.text.data(), message.Tam_text,
            message.tipoMensaje, message.identificador);

            //Asignar el nombre de fichero de Escritura
            std::string Fichero_Escritura = message.nombreArchivo.data();
            std::string Carpeta = "./ServerFile/";
            Fichero_Escritura = Carpeta + Fichero_Escritura;

            //Limpiamos el contenido del Archivo y le asignamos el nuevo Nombre
            message.nombreArchivo = {};
            Fichero_Escritura.copy(message.nombreArchivo.data(),Fichero_Escritura.length() ,0);

            // Comprobar tipo de mensaje y realizar una accion
            if(message.tipoMensaje == 1)
            {
                std::cout << "Primer Mensaje\n";
                //Crear Archivo
                int Write_File = open(message.nombreArchivo.data(), O_CREAT | O_WRONLY | O_TRUNC);
                std::cout << "fd_Wr: " << Write_File << std::endl;
                if(Write_File == -1)
                    std::cerr << "Error en la creacion y apertura del fichero\n";

                //Añadir elemento al set
                //auto it = receiving_data.begin();
                //receiving_data.insert(it,tupla);
                //Hacer write en Archivo
                message.text[1023] = '\0';
                DatosEscritos = write(Write_File, message.text.data(), message.Tam_text);
                std::cout << "--------DAT_ESC: " << DatosEscritos << std::endl;
            }
            else if(message.tipoMensaje == 2)
            {
                std::cout << "Mensaje Intermedio\n";
                //Buscar en el set el identificador
                //for(auto i : receiving_data)
                //{
                //    std::get<7>(tupla));
                //    std::cout << "ID_Tupla: " << std::get<7>(tupla)) << std::endl;
                //}
                //Write en ese archivo ya creado
                message.text[1023] = '\0';
                DatosEscritos = write(message.identificador, message.text.data(), message.Tam_text);
                std::cout << "--------DAT_ESC: " << DatosEscritos << std::endl;
            }
            else if(message.tipoMensaje == 3)
            {
                std::cout << "Ultimo Mensaje\n";
                //Write en el fichero
                DatosEscritos = write(message.identificador, message.text.data(), message.Tam_text);
                std::cout << "--------DAT_ESC: " << DatosEscritos << std::endl;
                //Cerrar fd
                close(message.identificador);
                //Eliminar del set la tupla afectada
            }
            else if(message.tipoMensaje == 4)
            {
                std::cout << "Mensaje Unico\n";
                //Crear Archivo
                int Write_Files = open(message.nombreArchivo.data(), O_CREAT | O_WRONLY | O_TRUNC);
                if(Write_Files == -1)
                    std::cerr << "Error en la creacion y apertura del fichero\n";
                //Write en el archivo
                message.text[1023] = '\0';
                DatosEscritos = write(Write_Files, message.text.data(), message.Tam_text);
                std::cout << "--------DAT_ESC: " << DatosEscritos << std::endl; 
                //Cerrar fd
                close(message.identificador);
            }

            // Primero convertimos la dirección IP como entero de 32 bits
            // en una cadena de texto.
            char* remote_ip = inet_ntoa(local_address.sin_addr);

            // Recuperamos el puerto del remitente en el orden adecuado para nuestra CPU
            int remote_port = ntohs(local_address.sin_port);

            //Asignamos los receiving_data
            /*std::set<std::tuple<uint32_t, in_port_t, std::string, int>> receiving_data;
            std::tuple<uint32_t, in_port_t, std::string, int> tupla;
            tupla = std::make_tuple(address.sin_addr.s_addr,address.sin_port,message.nombreArchivo.data(),message.tipoMensaje);

            auto it = receiving_data.begin();
            receiving_data.insert(it,tupla);*/

            // Imprimimos el mensaje y la dirección del remitente

            std::cout << "--Datos del Mensaje--------\n";
            std::cout << "Identificador: " << message.identificador << std::endl;
            std::cout << "Nombre: " << message.nombreArchivo.data() << std::endl;
            std::cout << "Tipo: " << message.tipoMensaje << std::endl;
            /*std::cout << "El sistema " << remote_ip << ":" << remote_port <<
            " envió el mensaje '" << message.text.data() << "'\n";*/
        }
        
          
        //Mostrar la linea

        //DatosEscritos = write(fd,message.text.data(),message.text.size()-1);

        //std::cout << DatosEscritos << std::endl;
    } catch(const std::exception& e)
    {
        eptr = std::current_exception();
    }
    //Se acabo este hilo
    quit = true;
}

void Quit(std::exception_ptr& eptr, std::string s)
{
    try
    {
        while(s != "quit")
        {
            std::cin >> s;
        }
    } catch(...)
    {
        eptr = std::current_exception();
    }

    //Se acabo este hilo
    quit = true;
}

int protected_main(int argc, char* argv[])
{
    //Ivocar a la funcion int_signal_handler para cada signal
    std::signal(SIGINT, &int_signal_handler);
    std::signal(SIGTERM, &int_signal_handler);
    std::signal(SIGHUP, &int_signal_handler);


    std::exception_ptr eptr1 {};
    std::exception_ptr eptr2 {};
    std::string str;

    std::set<std::pair<int, std::thread *>> active_threads;

    CommandLineArguments arguments(argc, argv);

    if(arguments.show_help)
        return 0;

    if(arguments.server_mode)
    {
        //Creamos nuestro primer hilo
        hiloServer = new std::thread(&Modo_Servidor, std::ref(eptr1),
        arguments.conn_port);
        //std::thread hiloQuit(&Quit, std::ref(eptr2), str);
        //Esperar a que termine un hilo
        while(!quit);

        //Matar los hilos
        request_cancellation(hiloServer);
        //request_cancellation(hiloQuit);
        
        hiloServer->join();
        //hiloQuit.join();
    }else
    {
        //Creamos nuestro primer hilo
        std::thread hiloClient(&Modo_Cliente, std::ref(eptr1)
        , arguments.conn_port, arguments.ip);
        //std::thread hiloQuit(&Quit, std::ref(eptr2), str);

        //Esperar a que termine un hilo
        while(!quit);

        //Matar los hilos
        request_cancellation(hiloClient);
        //request_cancellation(hiloQuit);
        
        hiloClient.join();
        //hiloQuit.join();
    }

    //Si hilo termino con una excepcion
    if(eptr1)
        std::rethrow_exception(eptr1);

    if(eptr2)
       std::rethrow_exception(eptr2); 
       
    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        return protected_main(argc, argv);
    }
    catch(std::system_error& e)
    {
        std::cerr << "mitalk" << ": " << e.what() << std::endl;
        return 2;
    }catch(...)
    {
        std::cout << "Error desconocido\n";
        return 99;
    }
}
