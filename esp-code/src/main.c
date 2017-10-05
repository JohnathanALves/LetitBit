/*
 * UART.c
 *
 * Created: 28/06/2016 15:59:23
 * Author : Jadsonlee.sa
 * 			Gustavo Marques
 * TXD = RXD (pino 2) e RXD = TXD (pino 3) e GND = GND...o ATMEGA328P alimentado.
 */

#ifndef F_CPU
 #define F_CPU 16000000UL
#endif

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

void post_request(char *req, long int params[7])
{
	char json[200];

	sprintf(json,
		"{\"accelx\":\"%ld\","
		" \"accely\":\"%ld\","
		" \"accelz\":\"%ld\","
		" \"gyrox\":\"%ld\","
		" \"gyroy\":\"%ld\","
		" \"gyroz\":\"%ld\","
		" \"temp\": \"%ld\"}",
		params[0],
		params[1],
		params[2],
		params[3],
		params[4],
		params[5],
		params[6]
	);

	sprintf(
		req,
		"POST /send HTTP/1.1\r\n"
		"Host: letitbit.herokuapp.com:80\r\n"
		//"Host: letitbit.herokuapp.com:80\r\n"
		"Connection: close\r\n"
		"Content-Length: %d\r\n"
		"Content-Type: application/json\r\n\r\n"
		"%s",
		strlen(json),
		json
	);
}

void sendData(long int params[7])
{
	char PostRequest[500];
    char DataToSend[300];

	// ESP send data code
	post_request(PostRequest, params);
	writeString("AT+CIPSTART=0,\"TCP\",\"letitbit.herokuapp.com\",80\r\n");
	_delay_ms(3000);

	// SEND BACK THE DATA
	// OK
	sprintf(DataToSend, "AT+CIPSEND=0,%d\r\n", strlen(PostRequest));
	writeString(DataToSend);
	_delay_ms(2000);

	// WRITE THE DATA IN A SOCKET
	// SEND
	sprintf(DataToSend, "%s", PostRequest);
	writeString(DataToSend);
	_delay_ms(3000);

	// CLOSE THE SOCKET
	// OK
	sprintf(DataToSend, "AT+CIPCLOSE=0\r\n");
	writeString(DataToSend);
}

int main(void)
{
	int read_count = 0;
	long int params[7];
	long int AccelX,
		     AccelY,
			 AccelZ,
			 Temperatura,
			 gyroX,
			 gyroY,
			 gyroZ;

	memset(params, 0, sizeof params);

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
    // MPU specific code

	Twi_Init();
	MPU6050_Init();

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

    //==============================================================================
    // ENABLE MUTIPLE CONNECTIONS
    writeString("AT+CIPMUX=1\r\n");
	_delay_ms(3000);

	read_count = 0;

    // SYSTEM LOOP
    while(1)
    {
		// MPU interaction specific code
		Twi_Start();
	    Twi_Write( MPU6050_ADDRESS );
	    Twi_Write( MPU6050_RA_ACCEL_XOUT_H );

	    _delay_us(20);

	    Twi_Start();
	    Twi_Write( MPU6050_ADDRESS | 1 );

	    AccelX = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
	    AccelY = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
	    AccelZ = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
	    Temperatura = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
	    gyroX = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
	    gyroY = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
	    gyroZ = ( TWI_ReadACK() << 8 ) | TWIReadNACK();

	    Twi_Stop();

	    Temperatura = Temperatura + 12421;
	    Temperatura = Temperatura / 340;

		params[0] += AccelX;
		params[1] += AccelY;
		params[2] += AccelZ;
		params[3] += gyroX;
		params[4] += gyroY;
		params[5] += gyroZ;
		params[6] += Temperatura;

		read_count++;

		if (read_count >= 10) {
			sendData(params);

			read_count = 0;
			memset(params, 0, sizeof params);
		}

		_delay_ms(1000);
    }
}
