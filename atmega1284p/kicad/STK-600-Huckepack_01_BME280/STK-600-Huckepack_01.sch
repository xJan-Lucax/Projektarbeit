EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "STK 600 Huckepack Platine BME280"
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
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x05_P2.54mm_Vertical" H 2650 3650 50  0001 C CNN
F 3 "~" H 2650 3650 50  0001 C CNN
	1    2650 3650
	1    0    0    -1  
$EndComp
Text Notes 2000 3500 0    50   ~ 0
Pin1: SCL\n
Wire Wire Line
	3850 3450 3850 3100
Wire Wire Line
	3850 3100 4450 3100
Wire Wire Line
	4000 3850 4000 2800
Wire Wire Line
	4200 2900 4200 3200
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
Text Label 3300 3450 0    50   ~ 0
SDA
Text Label 3400 3000 0    50   ~ 0
SCL
Text Label 3100 3850 0    50   ~ 0
VCC
Text Label 2400 4100 0    50   ~ 0
GND
Wire Wire Line
	4450 3100 4450 3550
Wire Wire Line
	4200 2900 4500 2900
$Comp
L Device:C C1
U 1 1 6199B174
P 3200 1300
F 0 "C1" H 3315 1346 50  0000 L CNN
F 1 "C" H 3315 1255 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 3238 1150 50  0001 C CNN
F 3 "~" H 3200 1300 50  0001 C CNN
	1    3200 1300
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 6199B73C
P 4950 1350
F 0 "C2" H 5065 1396 50  0000 L CNN
F 1 "C" H 5065 1305 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 4988 1200 50  0001 C CNN
F 3 "~" H 4950 1350 50  0001 C CNN
	1    4950 1350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4450 1300 4600 1300
Wire Wire Line
	4600 1200 4950 1200
Wire Wire Line
	3650 1300 3650 1150
Wire Wire Line
	3650 1150 3500 1150
Wire Wire Line
	4050 1600 4050 2150
Wire Wire Line
	4050 2800 4000 2800
Wire Wire Line
	4950 1500 4950 2150
Wire Wire Line
	4950 2150 4050 2150
Connection ~ 4050 2150
Wire Wire Line
	3200 1450 3200 2150
Wire Wire Line
	3200 2150 4050 2150
Wire Wire Line
	4600 1200 4600 1300
Wire Wire Line
	3650 1400 3500 1400
Wire Wire Line
	3500 1400 3500 1150
Connection ~ 3500 1150
Wire Wire Line
	3500 1150 3200 1150
Wire Wire Line
	4900 2200 5400 2200
Wire Wire Line
	5400 2200 5400 1200
Wire Wire Line
	5400 1200 4950 1200
Connection ~ 4950 1200
Wire Wire Line
	4050 2800 4050 2500
Wire Wire Line
	4050 2500 2800 2500
Wire Wire Line
	2800 2500 2800 1150
Wire Wire Line
	2800 1150 3200 1150
Connection ~ 3200 1150
Wire Wire Line
	4050 2150 4050 2300
Wire Wire Line
	4050 2300 4500 2300
Wire Wire Line
	4500 2300 4500 2900
$Comp
L Regulator_Linear:BD33GA5WEFJ U1
U 1 1 6198F142
P 4050 1300
F 0 "U1" H 4050 1542 50  0000 C CNN
F 1 "BD33GA5WEFJ" H 4050 1451 50  0000 C CNN
F 2 "Package_SO:HTSOP-8-1EP_3.9x4.9mm_P1.27mm_EP2.4x3.2mm" H 4050 1200 50  0001 C CNN
F 3 "http://rohmfs.rohm.com/en/products/databook/datasheet/ic/power/linear_regulator/bdxxga5wefj-e.pdf" H 4050 1200 50  0001 C CNN
	1    4050 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4450 1400 4600 1400
Wire Wire Line
	4600 1400 4600 1300
Connection ~ 4600 1300
Wire Wire Line
	2450 3000 2450 3450
$Comp
L Connector:Conn_01x04_Male J1
U 1 1 619AC274
P 5050 3300
F 0 "J1" H 5158 3581 50  0000 C CNN
F 1 "Conn_01x04_Male" H 5158 3490 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 5050 3300 50  0001 C CNN
F 3 "~" H 5050 3300 50  0001 C CNN
	1    5050 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5250 3200 4200 3200
Connection ~ 4200 3200
Wire Wire Line
	4200 3200 4200 4100
Wire Wire Line
	4900 3300 5250 3300
Wire Wire Line
	4900 2200 4900 2600
Wire Wire Line
	4450 3550 4800 3550
Wire Wire Line
	4900 3550 4900 3400
Wire Wire Line
	4900 3400 5250 3400
Wire Wire Line
	4700 3000 4700 3500
Wire Wire Line
	4700 3500 5250 3500
Wire Wire Line
	2450 3000 4650 3000
$Comp
L Device:R R1
U 1 1 619D13CF
P 4650 2750
F 0 "R1" H 4720 2796 50  0000 L CNN
F 1 "R" H 4720 2705 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 4580 2750 50  0001 C CNN
F 3 "~" H 4650 2750 50  0001 C CNN
	1    4650 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 619D1918
P 5000 2750
F 0 "R2" H 5070 2796 50  0000 L CNN
F 1 "R" H 5070 2705 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad0.98x0.95mm_HandSolder" V 4930 2750 50  0001 C CNN
F 3 "~" H 5000 2750 50  0001 C CNN
	1    5000 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4650 2900 4650 3000
Connection ~ 4650 3000
Wire Wire Line
	4650 3000 4700 3000
Wire Wire Line
	4650 2600 4900 2600
Connection ~ 4900 2600
Wire Wire Line
	4900 2600 4900 3300
Wire Wire Line
	5000 2600 4900 2600
Wire Wire Line
	4800 3550 4800 2950
Wire Wire Line
	4800 2950 5000 2950
Wire Wire Line
	5000 2950 5000 2900
Connection ~ 4800 3550
Wire Wire Line
	4800 3550 4900 3550
Text Label 5100 2200 0    50   ~ 0
VCC-3,3V
$EndSCHEMATC
