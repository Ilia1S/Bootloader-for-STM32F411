/*
 * bootload.h
 *
 *  Created on: Jan 11, 2023
 *      Author: ilia
 */

#ifndef BOOTLOAD_H_
#define BOOTLOAD_H_


void toggleBlueLed(void);
uint8_t outputData (void);
uint8_t checkCRC (void);
uint8_t eraseData (void);
void writeData (void);

#endif /* BOOTLOAD_H_ */
