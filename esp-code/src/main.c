/*
 * UART.c
 *
 * Created: 28/06/2016 15:59:23
 * Author : Jadsonlee.sa
 * 			Gustavo Marques
 * TXD = RXD (pino 2) e RXD = TXD (pino 3) e GND = GND...o ATMEGA328P alimentado.
 */

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

//==============================================================================
// ADVANCED SETTINGS
#define HOST            "192.168.1.105" // IP FOR CONNECTION
#define PORT            "1234"          // PORT FOR CONNECTION
#define SSID            "JG"            // NETWORK SSID
#define PASSWORD        "cabritinhos"   // NETWORK PASSWORD
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

//==============================================================================
//	Function: pinb13_toggle
//	Description: toggles pinb to show by LED (pinb5 = arduino board pin 13)
//	system state visual advice
//==============================================================================
void pinb13_toggle()
{
	PORTB ^= (1 << PB5);
}

void pinb13_up()
{
	PORTB |= (1 << PB5);
}

void pinb13_down()
{
	PORTB &= ~(1 << PB5);
}


int main(void)
{
    int ConnectionPort, DataLength;
    char DataToSend[40];
    char DataReceived[50];
	char ConnectToAp[60];
	char staIpAddr[]  = "192.168.0.108";
	char requireStr[] = "GET /send\r\n";

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

	DDRB  = (1<<DDB5);	// habilita o pinb (output)

    //==============================================================================
    // ESP RESET
    writeString("AT+RST\r\n");
    wait_sec(3);

    //==============================================================================
    // ESP MODE: station mode
    writeString("AT+CWMODE=1\r\n");
	pinb13_down();
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');
	pinb13_up();

    //==============================================================================
    // ESP MODE: softAP + station mode
    // CONNECT TO A NETWORK
	sprintf(ConnectToAp, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWORD);

	writeString(ConnectToAp);
	wait_sec(7);
	// pinb13_down();
    // do{
    //     readString(DataReceived);
    // }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');
	// pinb13_up();

	//==============================================================================
    // GET IP
    // writeString("AT+CIFSR\r\n");
	// pinb13_down();
    // do{
    //     readString(DataReceived);
    // }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');
	// pinb13_up();

    //==============================================================================
    // ENABLE MUTIPLE CONNECTIONS
	writeString("AT+CIPMUX=1\r\n");
	pinb13_down();
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');
	pinb13_up();

    ConnectionPort = 8080;
    // DataLength = 2;

	// OPEN TCP CONNETION WITH THE STATION
	// OK
	sprintf(DataToSend, "AT+CIPSTART=1,\"TCP\",\"%s\",\"%d\"\r\n", staIpAddr, ConnectionPort);
	writeString(DataToSend);
	pinb13_down();
	_delay_ms(7000);
	// do{
	// 	readString(DataReceived);
	// }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');
	pinb13_up();

    // SEND THE DATA
    // OK
    sprintf(DataToSend, "AT+CIPSEND=1,%d\r\n", strlen(requireStr) + 2);
    writeString(DataToSend);
	pinb13_down();
	_delay_ms(5000);
    // do{
    //     readString(DataReceived);
    // }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');
	pinb13_up();

    // WRITE THE DATA IN THE CREATED SOCKET
    // SEND OK
    sprintf(DataToSend, "%s\r\n", requireStr);
    writeString(DataToSend);
	pinb13_down();
    do{
        readString(DataReceived);
    }while(DataReceived[0] != 'S' && DataReceived[1] != 'E');
	pinb13_up();

    // CLOSE THE SOCKET
    // OK
    sprintf(DataToSend, "AT+CIPCLOSE=1\r\n");
    writeString(DataToSend);
	pinb13_down();
    do{
        readString(DataReceived);
        if(DataReceived[0] == 'E' && DataReceived[1] == 'R')
        {
            break;
        }
    }while(DataReceived[0] != 'O' && DataReceived[1] != 'K');
	pinb13_up();
        // }

    // }
}
