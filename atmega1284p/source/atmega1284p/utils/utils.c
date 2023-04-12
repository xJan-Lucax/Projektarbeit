/*
 * utils.c
 *
 * Created: 25.02.2021 14:20:28
 *  Author: Niklas Theis
 */ 

#include "utils.h"

void ConvertFloatToCharArray(char* output, float value)
{
	char *sign = (value < 0) ? "-" : "";
	float absVal = (value < 0) ? -value : value;

	int integer = absVal;
	float fraction = absVal - integer;
	int fractionInt = trunc(fraction * 100);

	sprintf (output, "%s%d.%02d", sign, integer, fractionInt);
}
