/*
 * bootload.c
 *
 *  Created on: Jan 11, 2023
 *      Author: ilia
 */

#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "usart.h"

#define HEX0 ":100000001FC0FECFFDCFFCCFFBCFFACFF9CFF8CF8B"
#define HEX1 ":10001000F7CFF6CFF5CFF4CFF3CFF2CFF1CFF0CFCC"
#define HEX2 ":06002000EFCFEECFEDCFA3"
#define HEX3 ":100040000FE50DBF04E00EBFC298BA9A04E018B3E2"
#define HEX4 ":10005000102718BB01D0FBCFB3ECA0E51197F1F747"
#define QL 5 //quantity of lines
#define LEN 44 //maximum line length
#define DL 33 //maximum data length + \0

static char sHEX[QL][LEN] = {HEX0, HEX1, HEX2, HEX3, HEX4};
static char buffer[100];
static uint8_t errorFlag = 0;

void toggleBlueLed(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
}
uint8_t outputData (void)
{
	sprintf(buffer, "\n\n\rReading data . . .\n\r");
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
	HAL_Delay(500);
	for (uint8_t j=0; j<QL; j++)
	{
		if ((sHEX[j][0] == ':') && (strlen(sHEX[j])%2))
		{
			sprintf(buffer, "\n\rLine %u - %s\n\r", j+1, sHEX[j]);
			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
			for (uint8_t i=1; i<strlen(sHEX[j]); i++)
			{
				if ((sHEX[j][i]>='A' && 'F'>=sHEX[j][i]) || (sHEX[j][i]>='0' && '9'>=sHEX[j][i]))
				{
					continue;
				}
				else
				{
					sprintf(buffer, "Error in line %u. Non-existent character %c\n\r", j+1, sHEX[j][i]);
					HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
					errorFlag = 1;
				}
				if (i>LEN-1)
				{
					sprintf(buffer, "Error in line %u. Number of characters cannot exceed %u\n\r", j+1, LEN-1);
					HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
					errorFlag = 1;
				}
			}
			uint8_t byteCountDec = (sHEX[j][1] - 48)*16 + (sHEX[j][2] - 48);
			sprintf (buffer, "Byte count in DEC = %u\n\r", byteCountDec);
			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
			if (byteCountDec>255)
			{
				sprintf(buffer, "Error in line %u. Byte count in DEC cannot exceed 255\n\r", j+1);
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
				errorFlag = 1;
			}
			char address[5];
			for (uint8_t i=0; i<4; i++)
			{
				address[i] = sHEX[j][3+i];
			}
			address[4] = '\0';
			sprintf(buffer, "Address = 0x%s\n\r", address);
			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
			if (sHEX[j][7]=='0' && sHEX[j][8]=='0')
			{
				sprintf(buffer, "Record type - Data\n\r");
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
			}
			else if (sHEX[j][7]=='0' && sHEX[j][8]=='1')
			{
				sprintf(buffer, "Record type - End Of File\n\r");
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
			}
			else if (sHEX[j][7]=='0' && sHEX[j][8]=='2')
			{
				sprintf(buffer, "Record type - Extended Segment Address\n\r");
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
			}
			else
			{
				sprintf(buffer, "Error in line %u. Non-existent record type\n\r", j+1);
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
				errorFlag = 1;
			}
			char data[DL];
			for (uint8_t i=0; i<byteCountDec*2; i++)
			{
				data[i] = sHEX[j][9+i];
			}
			data[byteCountDec*2] = '\0';
			sprintf(buffer, "Data - %s\n\r", data);
			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
		}
		else if (sHEX[j][0] != ':')
		{
			sprintf(buffer, "\n\rError in line %u. A line must start with : character\n\r", j+1);
			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
			errorFlag = 1;
		}
		else if (!(strlen(sHEX[j])%2))
		{
			sprintf(buffer, "\n\rError in line %u. Number of characters must be odd\n\r", j+1);
			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
			errorFlag = 1;
		}
	}
	sprintf(buffer, "\n\r");
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
	if (!errorFlag)
	{
		return 1;
	}
	else
		return 0;
}
uint8_t checkCRC (void)
{
	uint8_t successfullCheck = 0;
	for (uint8_t j=0; j<QL; j++)
	{
		uint8_t sDecLength = strlen(sHEX[j])-1;
		uint8_t sDEC[LEN];
		for (uint8_t i=1; i<sDecLength+1; i++)
		{
			if (sHEX[j][i]>='A' && 'F'>=sHEX[j][i])
			{
				sDEC[i-1]=sHEX[j][i]-55;
			}
			else if (sHEX[j][i]>='0' && '9'>=sHEX[j][i])
			{
				sDEC[i-1]=sHEX[j][i]-48;
			}
		}
		uint16_t crcDec = 0; //sum of all pairs in DEC
		uint8_t crc = 0; //CRC in DEC
		for (uint8_t i=0; i<sDecLength-2; i+=2)
		{
			crcDec+= sDEC[i]*16 + sDEC[i+1];
		}
		crc = sDEC[sDecLength-2]*16 + sDEC[sDecLength-1];
		crcDec = crcDec & 255; //discard extra bits
		uint16_t check = (crc + crcDec) & 255;
		if (!check)
		{
			successfullCheck++;
			sprintf(buffer, "CRC line %u is OK!\t", j+1);
			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
		}
		else
		{
			sprintf(buffer, "CRC line %u is incorrect :-(\t", j+1);
			HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
		}
	}
	sprintf(buffer, "\n\r");
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
	if (successfullCheck == QL)
	{
		sprintf(buffer, "\n\rAll CRC are OK!\n\r");
		HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
		return 1;
	}
	else
		return 0;
}
uint8_t eraseData (void)
{
	uint32_t sError;
	sprintf(buffer, "\n\rErase data from flash . . .\n\r");
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
	HAL_Delay(500);
	HAL_FLASH_Unlock();
	static FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.Sector = FLASH_SECTOR_1;
	EraseInitStruct.NbSectors = 2;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &sError) != HAL_OK)
	{
		sprintf(buffer, "\n\rErase error :-(\n\r");
		HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
		HAL_FLASH_Lock();
		HAL_Delay(500);
		return 0;
	}
	else
	{
		sprintf(buffer, "\n\rErase completed successfully!\n\r");
		HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
		HAL_FLASH_Lock();
		HAL_Delay(500);
		return 1;
	}
}
void writeData()
{
	sprintf(buffer, "\n\rWriting data to flash . . .\n\r");
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
	HAL_Delay(500);
	HAL_FLASH_Unlock();
	for (uint8_t j=0; j<QL; j++)
	{
		uint32_t startAddress = 0x08004000U, shift = 0;
		uint8_t sDataDECLength = 0, sDataHEXLength = 0;
		sDataDECLength = (sHEX[j][1] - 48)*16 + (sHEX[j][2] - 48);
		sDataHEXLength = sDataDECLength*2;
		char data[DL];
		uint8_t sDataHEX[LEN-1]; //auxiliary array for conversion from 16 to 10 number system
		uint8_t sDataDEC[(LEN-1)/2]; //array of data in DEC
		for (uint8_t i=0; i<sDataHEXLength; i++)
		{
			data[i] = sHEX[j][9+i];
		}
		data[sDataHEXLength] = '\0';
		for (uint8_t i=0; i<sDataHEXLength; i++)
		{
			if (data[i]>='A' && 'F'>=data[i])
			{
				sDataHEX[i]=data[i]-55;
			}
			else if (data[i]>='0' && '9'>=data[i])
			{
				sDataHEX[i]=data[i]-48;
			}
		}
		shift = (sHEX[j][5]-48)*16*16*16 + (sHEX[j][6]-48)*16*16 + (sHEX[j][7]-48)*16 + (sHEX[j][8]-48);
		startAddress += shift;
		for (uint8_t i=0; i<sDataDECLength; i++)
		{
			sDataDEC[i] = sDataHEX[i*2]*16 + sDataHEX[i*2+1];
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, startAddress, sDataDEC[i]);
			startAddress++;
		}
		__IO uint8_t checkWriting = 0; //writing check
		startAddress = 0x08004000U;
		for (uint8_t i = 0; i<sDataDECLength; i++, shift++)
		{
			checkWriting = *(__IO uint8_t *)(startAddress+shift);
			if (checkWriting == sDataDEC[i])
			{
				continue;
			}
			else
			{
				sprintf(buffer, "\n\rWriting error :-(\n\r");
				HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
				HAL_FLASH_Lock();
				return;
			}
		}
	}
	sprintf(buffer, "\n\rWriting completed successfully!\n\r");
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), 100);
	HAL_FLASH_Lock();
	HAL_Delay(500);
}
