/*
 * CRC.h
 *
 * Created: 18.02.2021 16:48:17
 *  Author: Marvin
 * http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html 18.02.2021
 */ 
#pragma once

#include <avr/io.h>
#define CRC_POLY  0x31 // CRC polynomial 0x31 = x^8 + x^5 + x^4 + 1)
#define CRC_INIT  0xFF // Initial value

const uint8_t crcLookupTable_u8[256];
uint8_t calc_CRC_8(uint16_t value);
//void calc_Table_CRC_8();