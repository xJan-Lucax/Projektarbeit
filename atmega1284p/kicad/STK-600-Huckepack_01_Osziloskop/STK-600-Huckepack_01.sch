EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "STK 600 Huckepack Platine I2C für Oszilloskop"
Date "2021-11-20"
Rev "1.0"
Comp "FH Bielefeld"
Comment1 "Philipp Husemann"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_02x05_Odd_Even PORT_A1
U 1 1 618D8315
P 2650 3650
F 0 "PORT_A1" H 2700 4067 50  0000 C CNN
F 1 "Conn_02x05_Odd_Even_MountingPin" H 2700 3976 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x05_P2.54mm_Vertical" H 2650 3650 50  0001 C CNN
F 3 "~" H 2650 3650 50  0001 C CNN
	1    2650 3650
	1    0    0    -1  
$EndComp
Text Notes 2000 3500 0    50   ~ 0
Pin1: SCL\n
Wire Wire Line
	2450 3000 2450 3450
Wire Wire Line
	4000 3850 4000 2800
Wire Wire Line
	2250 4100 2250 3850
Wire Wire Line
	2250 3850 2450 3850
NoConn ~ 2450 3550
NoConn ~ 2450 3650
NoConn ~ 2450 3750
NoConn ~ 2950 3750
NoConn ~ 2950 3650
NoConn ~ 2950 3550
NoConn ~ 6350 -50 
Wire Wire Line
	2950 3450 3850 3450
Wire Wire Line
	2250 4100 4200 4100
Wire Wire Line
	2950 3850 4000 3850
Text Label 3600 3450 0    50   ~ 0
SDA
Text Label 3500 4100 0    50   ~ 0
GND
Text Label 3550 3850 0    50   ~ 0
VDD
$Comp
L power:+5V #PWR01
U 1 1 61991414
P 4000 2800
F 0 "#PWR01" H 4000 2650 50  0001 C CNN
F 1 "+5V" H 4015 2973 50  0000 C CNN
F 2 "" H 4000 2800 50  0001 C CNN
F 3 "" H 4000 2800 50  0001 C CNN
	1    4000 2800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 6199231F
P 4200 4100
F 0 "#PWR02" H 4200 3850 50  0001 C CNN
F 1 "GND" H 4205 3927 50  0000 C CNN
F 2 "" H 4200 4100 50  0001 C CNN
F 3 "" H 4200 4100 50  0001 C CNN
	1    4200 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3850 3450 3850 3100
Text Label 3600 3000 0    50   ~ 0
SCL
Wire Wire Line
	2450 3000 3600 3000
Text GLabel 3850 3100 0    50   Input ~ 0
SDA
Text GLabel 3600 3000 0    50   Input ~ 0
SCL
$EndSCHEMATC
