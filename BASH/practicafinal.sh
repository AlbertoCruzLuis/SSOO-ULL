#==============================================================
#@Autor: Alberto Cruz Luis
#@Email: alu0101217734@ull.edu.es
#@Fecha: Noviembre 2019
#@Name: practicafinal.sh
#@Version: Practica BASH
#==============================================================

#!/bin/bash

# PRACTICA FINAL - Control de los ficheros abiertos por los usuarios.

#Variables

USUARIOS=$(ps -A --no-headers -ouser | sort | uniq)
CHECK="0"
ON="0"
OFF="0"

##### Estilos
TEXT_ULINE=$(tput sgr 0 1)
TEXT_BOLD=$(tput bold)

TEXT_GREEN=$(tput setaf 2)
TEXTBG_RED=$(tput setab 0)
TEXT_BLACK=$(tput setaf 1)
TEXT_BLUE=$(tput setaf 4)
TEXT_RESET=$(tput sgr0)

##### Funciones

Lista_Procesos()
{
    echo ${TEXT_BOLD}${TEXTBG_RED}${TEXT_BLACK}$TITLE$TEXT_RESET

	printf "${TEXT_BOLD}${TEXT_BLUE}USUARIOS\t\tUID\tGID\tCOMANDO\t\t\t\tTIEMPO\t  ANTIGUO\tEJECUCION\tFICHEROS\tPROCESOS\n$TEXT_RESET"

	for i in $USUARIOS ; do

	U_ID=$(id -u $i) 2> /dev/null
	G_ID=$(id -g $i) 2> /dev/null
	COMANDOCPU=$(ps -u $i -otime,cmd --no-headers | sort | tail -n1 | awk '{ print $2}') 2> /dev/null
	TIEMPO=$(ps -u $i -otime,cmd --no-headers | sort | tail -n1 | awk '{ print $1}') 2> /dev/null
	ANTIGUO=$(ps -u $i -ostart,time --no-headers | sort | head -n1 | awk '{ print $1}') 2> /dev/null
	EJECUCION=$(ps -u $i -ostart,time --no-headers | sort | head -n1 | awk '{ print $2}') 2> /dev/null
	FICHEROS=$(lsof -u $i | wc -l) 2> /dev/null
	PROCESOS=$(ps -u $i | wc -l) 2> /dev/null
		printf "%-23s %-7d %-7d %-31s %-9s %-13s %-15s %-15d %-10d\n" "$i" "$U_ID" "$G_ID" "$COMANDOCPU" "$TIEMPO" "$ANTIGUO" "$EJECUCION" "$FICHEROS" "$PROCESOS"
	done
}

Error_Exit()
{
    echo "$1" 1>&2
    exit 1
}

##### Manual de Uso
usage()
{
    printf "\t\tMANUAL DE USO\n"
    printf "SYNOPSIS\n"
    printf "\t $0 [OPCIONES]\n\n"

    printf "DESCRIPCION\n"
    printf "\tMuestra una Tabla Con diversas funciones de control de ficheros\n"

    printf "OPCIONES\n"
    printf "\t[-ON] [--online]       Muestra la informacion de los usuarios que estan conectados al sistema\n"
    printf "\t[-OFF] [--offline]     Muestra la informacion de los usuarios que no estan conectados al sistema\n"
    printf "\t[-U] [--uid]           Ordenar por el Identificador de Usuario(UID)\n"
    printf "\t[-R] [--reverse]       Ordenar inversamente\n"
    printf "\t[-k N] [--kill files]  Elimina los procesos con un número de ficheros abiertos superior al indicado\n"
}

#Comprobacion de los comandos necesarios para ejecutar el script
Comprobacion_Comandos()
{
    test -x "$(which ps)" || Error_Exit "El comando <ps> no se puede ejcutar"
    test -x "$(which who)" || Error_Exit "El comando <who> no se puede ejcutar"
    test -x "$(which awk)" || Error_Exit "El comando <awk> no se puede ejcutar"
    test -x "$(which sed)" || Error_Exit "El comando <sed> no se puede ejcutar"
    test -x "$(which printf)" || Error_Exit "El comando <printf> no se puede ejcutar"
    test -x "$(which uniq)" || Error_Exit "El comando <uniq> no se puede ejcutar"
    test -x "$(which lsof)" || Error_Exit "El comando <lsof> no se puede ejcutar"
    test -x "$(which id)" || Error_Exit "El comando <id> no se puede ejcutar"
    test -x "$(which head)" || Error_Exit "El comando <head> no se puede ejcutar"
    test -x "$(which tail)" || Error_Exit "El comando <tail> no se puede ejcutar"
    test -x "$(which wc)" || Error_Exit "El comando <wc> no se puede ejcutar"
}


#Ejecutamos la funcion Comprobacion_Comandos para realizar las correspondientes comprobaciones  
Comprobacion_Comandos


#Comprobar si no le pasamos parametros al script
if [ "$1" == "" ]; then
    USUARIOS=$(ps -A --no-headers -ouser | sort | uniq) 2> /dev/null
    Lista_Procesos
fi

while [ "$#" -gt 0 ]; do
   case $1 in
       -h | --help )

            usage
            CHECK=1
           ;;
	    -ON | --online )

            USUARIOS=$(who | sort | uniq | awk '{print $1}') 2> /dev/null
            ON=1

           ;;
	    -OFF | --offline )

            USUARIOS=$(ps -A --no-headers -ouser | sort | uniq | sed "s/$(who | sort | awk '{ print $1 }')//g" | sort | uniq | sed "1d") 2> /dev/null
            OFF=1
           ;;
        -U | --uid )

            USUARIOS=$( ps -A --no-headers -ouid,user | sort -n | uniq | awk '{ print $2 }') 2> /dev/null
           ;;
        -R | --reverse )

            USUARIOS=$(ps -A --no-headers -ouser | sort | uniq | sort -r) 2> /dev/null
            ;;
        -k | --kill )

                echo "Kill N"            
           ;;
       * )
            CHECK=1
            Error_Exit "OPCION NO SOPORTADA - Para mas informacion $0 -h"
   esac

    #Combrobacion de comandos no compatibles a la vez
    if [ "$ON" == "1" ] && [ "$OFF" == "1" ]; then
        CHECK=1
        Error_Exit "No se pueden usar las opciones -ON y -OFF al mismo tiempo"
    fi

    if [ $# -eq 1 ] && [ "$CHECK" -ne "1" ]; then
        Lista_Procesos
    fi

    shift
done
