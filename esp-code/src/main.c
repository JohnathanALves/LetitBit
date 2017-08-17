/*
 * UART.c
 *
 * Created: 28/06/2016 15:59:23
 * Author : Jadsonlee.sa
 * 			Gustavo Marques
 * TXD = RXD (pino 2) e RXD = TXD (pino 3) e GND = GND...o ATMEGA328P alimentado.
 */

#define F_CPU 16000000UL
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

//==============================================================================
// ADVANCED SETTINGS
#define HOST            "192.168.1.105" // IP FOR CONNECTION
#define PORT            "1234"          // PORT FOR CONNECTION
#define SSID            "Marques"       // NETWORK SSID
#define PASSWORD        "M12345678"     // NETWORK PASSWORD
#define MAX_DATA        5               // MAX NUMBER OF 'UPLOAD'
#define MAX_TRY         3               // MAX NUMBER OF FAILS CONNECTIONS


//==============================================================================
//	Function: getByte
//	Description:
//==============================================================================
char getByte()
{
    while((UCSR0A & 0x80) != 0x80){}; //Espera a recepção de dados.
                                      //RXC0 é zerado após a leitura de UDR0
    return UDR0;//Armazena os dados recebidos em data.
}

//==============================================================================
//	Function: writeString
//	Description:
//==============================================================================
void readString(char *str)
{
    int n=0;
    str[n] = '\0';
    do
    {
        str[n] = getByte();
    }
    while (str[n++] != '\n');
}

//==============================================================================
//	Function: sendByte
//	Description:
//==============================================================================
void sendByte(unsigned char data)
{
	while((UCSR0A & 0x20) != 0x20){}//Verifica se UDR0 pode receber novos dados para Tx.									//
	UDR0 = data;
	while((UCSR0A & 0x40) != 0x40){}//Verifica se os dados já foram "transmitidos".
	UCSR0A = 0x40;//Para zerar TXC0 (bit 6).

}

//==============================================================================
//	Function: writeString
//	Description:
//==============================================================================
void writeString(const char *str)
{
	while (*str != '\0')
	{
		sendByte(*str);
		++str;
	}
}

//==============================================================================
//	Function: wait_sec
//	Description:
//==============================================================================
void wait_sec(int n)
{
    int i;
    for(i=0;i<1000;i++)
    {
        _delay_ms(n);
    }
}

int main(void)
{
    char ConnectionPort, DataLength;
    char DataToSend[40];
    char DataReceived[50];

    //==============================================================================
    // CONFIG SERIAL PORT

	//Para Fosc = 16M, U2Xn = 0 e um baud rate = 9600, temos que UBRR0 = 103d = 00000000 01100111
	//Para um baud rate = 115200, temos UBRR0 = 8d = 00000000 00001000
	UBRR0H = 0b00000000;//115200 bits/s.
	UBRR0L = 0b00001000;

	UCSR0A = 0b01000000; //TXC0 = 1 (zera esse flag), U2X0 = 0 (velocidade normal),
						 //MPCM0 = 0 (desabilita modo multiprocessador).
	UCSR0B = 0b00011000; //Desabilita interrupção recepção (RXCIE0 = 0), transmissão (TXCIE0 = 0)
						 //e buffer vazio (UDRIE0=0), habilita receptor USART RXEN0 = 1
						 //(RxD torna-se uma entrada), habilita transmissor TXEN0 = 1
						 //(TxD torna-se uma saída), seleciona 8 bits de dados (UCSZ2 = 0).
	UCSR0C = 0b00000110; //Habilita o modo assíncrono (UMSEL01/00 = 00), desabilita paridade (UPM01/00 = 00),
						 //Seleciona um bit de stop (USBS0 = 0), seleciona 8 bits de dados (UCSZ1/0 = 11) e
						 //sem polaridade (UCPOL0 = 0 - modo assíncrono).

    //==============================================================================
    // ESP RESET
    writeString("AT+RST\r\n");
    wait_sec(3);

    //==============================================================================
    // ESP MODE: softAP + station mode
    writeString("AT+CWMODE_CUR=3\r\n");
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');

    //==============================================================================
    // ESP MODE: softAP + station mode
    // CONNECT TO A NETWORK
    writeString("AT+CWJAP_CUR=\"Brega Familiar\",\"boapergunta\"\r\n");
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');

    // CREATE A NETWORK
    writeString("AT+CWSAP_CUR=\"ESP8266\",\"1234567890\",5,3\r\n");
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');



    //==============================================================================
    // GET IP
    writeString("AT+CIFSR\r\n");
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');

    //==============================================================================
    // ENABLE MUTIPLE CONNECTIONS
    writeString("AT+CIPMUX=1\r\n");
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');

    //==============================================================================
    // SET THE SERVER TIMEOUT
    /*writeString("AT+CIPSTO=10\r\n");
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K'); */

    //==============================================================================
    // ENABLE WEB SERVER
    writeString("AT+CIPSERVER=1\r\n");
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');


    //==============================================================================
    // SYSTEM LOOP
    while(1)
    {
        readString(DataReceived);

        // NEW CONNECTION
        if(DataReceived[2] == 'C' && DataReceived[3] == 'O')
        {
            //writeString("NEW CONNECTION\r\n");
        }

        // DISCONNECTION
        if(DataReceived[2] == 'C' && DataReceived[3] == 'L')
        {
            //writeString("NEW DISCONNECTION\r\n");
        }

        // RECEIVED COMMAND
        if(DataReceived[0] == '+' && DataReceived[1] == 'I')
        {

            ConnectionPort = 0;
            DataLength = 2;

            // SEND BACK THE DATA
            // OK
            sprintf(DataToSend, "AT+CIPSEND=%c,%d\r\n", DataReceived[5], 15);
            writeString(DataToSend);
            do{
                readString(DataReceived);
            }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');

            // WRITE THE DATA IN A SOCKET
            // SEND
            sprintf(DataToSend, "V:%02d T:%02d T:%02d ", 99, 35, 50);
            writeString(DataToSend);
            do{
                readString(DataReceived);
            }while(DataReceived[0] != 'S' && DataReceived[1] != 'E');

            // CLOSE THE SOCKET
            // OK
            sprintf(DataToSend, "AT+CIPCLOSE=%c\r\n", DataReceived[5]);
            writeString(DataToSend);
            do{
                readString(DataReceived);
                if(DataReceived[0] == 'E' && DataReceived[1] == 'R')
                {
                    break;
                }
            }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');
        }

    }
}