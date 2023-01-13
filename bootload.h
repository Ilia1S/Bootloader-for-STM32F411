/*
 * bootload.h
 *
 *  Created on: Jan 11, 2023
 *      Author: ilia
 */

#ifndef BOOTLOAD_H_
#define BOOTLOAD_H_


void toggleBlueLed(void);
void outputData (void);
void checkCRC (void);
void eraseData (void);
void writeData (void);

#endif /* BOOTLOAD_H_ */
