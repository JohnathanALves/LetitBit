/*
 * Testando.c
 *
 * Created: 24/08/2017
 * Author : Gustavo Marques
 */

//#define __AVR_ATmega328__

#ifndef F_CPU
 #define F_CPU 16000000UL
#endif

#include "mpu6050.h"

//  _____ _    _ _____
// |_   _| |  | |_   _|
// | | | |  | | | |
// | | | |/\| | | |
// | | \  /\  /_| |_
// \_/  \/  \/ \___/
//============================================================================
// INITIAL CONFIG
void Twi_Init(void)
{
    //set SCL to 400kHz
    TWSR = 0x00;
    TWBR = 0x0C;
    //enable TWI
    TWCR = (1<<TWEN);
}

//============================================================================
//SEND START SIGNAL
void Twi_Start(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

//============================================================================
//SEND STOP SIGNAL
void Twi_Stop(void)
{
    //TWCR = 0xC5;
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

//============================================================================
//SEND WRITE SIGNAL
void Twi_Write(uint8_t u8data)
{
    TWDR = u8data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

//============================================================================
//SEND READ SIGNAL
uint8_t TWI_ReadACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}
//read byte with NACK
uint8_t TWIReadNACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

//============================================================================
//
uint8_t TWIGetStatus(void)
{
    uint8_t status;
    //mask status
    status = TWSR & 0xF8;
    return status;
}

//  _   _ _____  ___  ______ _____
// | | | /  ___|/ _ \ | ___ \_   _|
// | | | \ `--./ /_\ \| |_/ / | |
// | | | |`--. \  _  ||    /  | |
// | |_| /\__/ / | | || |\ \  | |
// \___/\____/\_| |_/\_| \_| \_/
//------------------------------------------------------------------------------

void USART_Init()
{
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
}

//==============================================================================
//	Function: getByte
//	Description:
//==============================================================================
char USART_getByte()
{
    while((UCSR0A & 0x80) != 0x80){}; //Espera a recepção de dados.
                                      //RXC0 é zerado após a leitura de UDR0
    return UDR0;//Armazena os dados recebidos em data.
}

//==============================================================================
//	Function: writeString
//	Description:
//==============================================================================
void USART_readString(char *str)
{
    int n=0;
    str[n] = '\0';
    do
    {
        str[n] = USART_getByte();
    }
    while (str[n++] != '\n');
}

//==============================================================================
//	Function: sendByte
//	Description:
//==============================================================================
void USART_sendByte(unsigned char data)
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
void USART_writeString(const char *str)
{
	while (*str != '\0')
	{
		USART_sendByte(*str);
		++str;
	}
}

// ___  _________ _   _
// |  \/  || ___ \ | | |
// | .  . || |_/ / | | |
// | |\/| ||  __/| | | |
// | |  | || |   | |_| |
// \_|  |_/\_|    \___/
//============================================================================

void MPU6050_Init()
{
  Twi_Start();
  Twi_Write( MPU6050_ADDRESS );
  Twi_Write( MPU6050_RA_PWR_MGMT_1 );
  Twi_Write( 2 ); //Sleep OFF          // 0b 0000 0010
  Twi_Stop();

  _delay_ms(1);

  Twi_Start();
  Twi_Write( MPU6050_ADDRESS );
  Twi_Write( MPU6050_RA_GYRO_CONFIG );
  Twi_Write( 0 );
  Twi_Stop();

  _delay_ms(1);

  Twi_Start();
  Twi_Write( MPU6050_ADDRESS );
  Twi_Write( MPU6050_RA_ACCEL_CONFIG );
  Twi_Write( 0 );
  Twi_Stop();

  Twi_Start();
  Twi_Write( MPU6050_ADDRESS );
  Twi_Write( MPU6050_RA_SMPLRT_DIV );
  Twi_Write( 5 );
  Twi_Stop();

}

//============================================================================
//
// int main(void)
// {
//   long int status,i;
//   long int AccelX,AccelY,AccelZ, Temperatura, gyroX,gyroY,gyroZ;
//   char Data[30];
//
//
//   USART_Init();
//   //USART_writeString("Started!\r\n");
//   //USART_writeString("USART CHECK!\r\n");
//   Twi_Init();
//   //USART_writeString("I2C CHECK!\r\n");
// 	MPU6050_Init();
//   //USART_writeString("MPU CHECK!\r\n");
//
//   //USART_writeString("READING!\r\n");
//   _delay_ms(5000);
//   //============================================================================
//   while (1)
//   {
//     Twi_Start();
//     Twi_Write( MPU6050_ADDRESS );
//     Twi_Write( MPU6050_RA_ACCEL_XOUT_H );
//     _delay_us(20);
//     Twi_Start();
//     Twi_Write( MPU6050_ADDRESS | 1 );
//     AccelX = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
//     AccelY = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
//     AccelZ = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
//     Temperatura = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
//     gyroX = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
//     gyroY = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
//     gyroZ = ( TWI_ReadACK() << 8 ) | TWIReadNACK();
//     Twi_Stop();
//
//     Temperatura = Temperatura + 12421;
//     Temperatura = Temperatura / 340;
//
//     gyroX += 697;
//     //gyroY += 655;
//     //gyroZ += 75;
//
//     //USART_sendByte(0xff);
//
//     sprintf(Data, "%06ld,%06ld,%06ld,", AccelX, AccelY, AccelZ);
//     USART_writeString(Data);
//     sprintf(Data, "%06ld,%06ld,%06ld,", gyroX, gyroY, gyroZ);
//     USART_writeString(Data);
//     sprintf(Data, "%06ld,\n", Temperatura);
//     USART_writeString(Data);
//
//     _delay_ms(1);
//
//   }
// }
