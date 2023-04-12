EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "STK 600 Huckepack Platine SCD30"
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
$Comp
L tinkerforge:SCD30 U1
U 1 1 61924348
P 4800 3100
F 0 "U1" H 5178 3146 50  0000 L CNN
F 1 "SCD30" H 5178 3055 50  0000 L CNN
F 2 "AA-Z:SCD30" H 4850 3000 50  0001 C CNN
F 3 "" H 4850 3000 50  0001 C CNN
	1    4800 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 3000 2450 3450
Wire Wire Line
	3850 3450 3850 3100
Wire Wire Line
	3850 3100 4450 3100
Wire Wire Line
	4000 3850 4000 2800
Wire Wire Line
	4000 2800 4450 2800
Wire Wire Line
	4450 2900 4200 2900
Wire Wire Line
	4200 2900 4200 4100
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
NoConn ~ 4450 3400
NoConn ~ 4450 3300
NoConn ~ 4450 3200
NoConn ~ 6350 -50 
Wire Wire Line
	2450 3000 4450 3000
Wire Wire Line
	2950 3450 3850 3450
Wire Wire Line
	2250 4100 4200 4100
Wire Wire Line
	2950 3850 4000 3850
$EndSCHEMATC
