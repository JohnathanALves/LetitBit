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
#include "mpu6050.h"

#define USART_BAUDRATE 19200
#define UBBR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

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

void post_request(char *req, int params[6])
{
	sprintf(
		req,
		"POST /send HTTP/1.1\r\n"
		"Host: letitbit.herokuapp.com:80\r\n"
		"Content-Type: application/json\r\n\r\n"
		// "Cache-Control: no-cache\r\n\rn"
		"{\"x\":1}"
		,
		params[0],
		params[1],
		params[2],
		params[3],
		params[4],
		params[5]
	);
}

int main(void)
{
    char ConnectionPort, DataLength;
    char DataToSend[40];
    char DataReceived[50];

	// char GetRequest[] = "GET / HTTP/1.1\r\n"
	// 					"Host: 10.42.0.21:8080\r\n\r\n";
						// "Cache-Control: no-cache\r\n\r\n";

    //==============================================================================
    // CONFIG SERIAL PORT

	//Para Fosc = 16M, U2Xn = 0 e um baud rate = 9600, temos que UBRR0 = 103d = 00000000 01100111
	//Para um baud rate = 115200, temos UBRR0 = 8d = 00000000 00001000
	UBRR0H = 0b00000000;//115200 bits/s.
	UBRR0L = 0b00001000;

	// UBRR0H = (uint8_t)(UBBR_VALUE>>8);
	// UBRR0L = (uint8_t)UBBR_VALUE;

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
    // ESP DISABLE ECHO
	// OK
	 writeString("ATE1\r\n");
	 _delay_ms(3000);

    //==============================================================================
    // ESP MODE: softAP + station mode
    writeString("AT+CWMODE_CUR=3\r\n");
	_delay_ms(3000);

    //==============================================================================
    // ESP MODE: softAP + station mode
    // CONNECT TO A NETWORK
    writeString("AT+CWJAP_CUR=\"OnePlus3\",\"12345678\"\r\n");
	_delay_ms(10000);

    // CREATE A NETWORK
    // writeString("AT+CWSAP_CUR=\"ESP8266\",\"12345678\",5,3\r\n");
	// _delay_ms(6000);

    //==============================================================================
    // ENABLE MUTIPLE CONNECTIONS
    writeString("AT+CIPMUX=1\r\n");
	_delay_ms(6000);

	//==============================================================================
   // // ENABLE WEB SERVER
    // writeString("AT+CIPSERVER=1\r\n");
	// _delay_ms(6000);

    // SYSTEM LOOP
    while(1)
    {
		char PostRequest[500];
		int params[] = {1,2,3,4,5,6};
		// ConnectionPort = 0;
        // DataLength = 2;

		post_request(PostRequest, params);

		writeString("AT+CIPSTART=0,\"TCP\",\"letitbit.herokuapp.com\",80\r\n");
		_delay_ms(5000);

		// printf("%s\n", PostRequest);
		// _delay_ms(3000);

        // SEND BACK THE DATA
        // OK
        sprintf(DataToSend, "AT+CIPSEND=0,%d\r\n", strlen(PostRequest));
        writeString(DataToSend);
		_delay_ms(3000);

        // WRITE THE DATA IN A SOCKET
        // SEND
        sprintf(DataToSend, "%s", PostRequest);
        writeString(DataToSend);
		_delay_ms(14000);

        // CLOSE THE SOCKET
        // OK
        sprintf(DataToSend, "AT+CIPCLOSE=0\r\n");
        writeString(DataToSend);

        _delay_ms(5000);
    }
}
