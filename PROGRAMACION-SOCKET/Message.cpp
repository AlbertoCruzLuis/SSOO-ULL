#include <array>

struct Message {

	int identificador;					// Identificador para el cliente
	std::array<char, 50> nombreArchivo;	// Nombre del archivo en el cliente
   	int tipoMensaje;  	/// Tipo de mensaje que se está enviando 
                     	// Primer mensaje 1
                     	// Mensaje intermedio 2
                     	// Ultimo mensaje 3
                     	// Mensaje único 4


	//...						// Otros campos del mensaje

	std::array<char, 1024> text;			// Igual que "char text[1024]"
	int Tam_text;					//Tamaño del mensaje leido
							// pero recomendado así en C++

	//...						// Otros campos del mensaje

};
