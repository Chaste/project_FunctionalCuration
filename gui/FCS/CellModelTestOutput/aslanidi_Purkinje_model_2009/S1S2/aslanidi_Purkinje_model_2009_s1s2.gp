# Gnuplot script file generated by Chaste.
# First plot is of the actual action potentials overlaid on one another.
set terminal postscript eps size 3, 2
set output "/export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/aslanidi_Purkinje_model_2009_s1s2.eps"
set title "Aslanidi Purkinje Model 2009"
set xlabel "Time (ms)"
set ylabel "Voltage (mV)"
set grid
set autoscale
set style data lines
set key off"
plot "< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 1:92 with lines title "1",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 2:93 with lines title "2",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 3:94 with lines title "3",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 4:95 with lines title "4",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 5:96 with lines title "5",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 6:97 with lines title "6",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 7:98 with lines title "7",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 8:99 with lines title "8",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 9:100 with lines title "9",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 10:101 with lines title "10",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 11:102 with lines title "11",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 12:103 with lines title "12",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 13:104 with lines title "13",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 14:105 with lines title "14",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 15:106 with lines title "15",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 16:107 with lines title "16",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 17:108 with lines title "17",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 18:109 with lines title "18",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 19:110 with lines title "19",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 20:111 with lines title "20",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 21:112 with lines title "21",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 22:113 with lines title "22",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 23:114 with lines title "23",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 24:115 with lines title "24",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 25:116 with lines title "25",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 26:117 with lines title "26",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 27:118 with lines title "27",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 28:119 with lines title "28",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 29:120 with lines title "29",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 30:121 with lines title "30",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 31:122 with lines title "31",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 32:123 with lines title "32",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 33:124 with lines title "33",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 34:125 with lines title "34",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 35:126 with lines title "35",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 36:127 with lines title "36",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 37:128 with lines title "37",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 38:129 with lines title "38",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 39:130 with lines title "39",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 40:131 with lines title "40",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 41:132 with lines title "41",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 42:133 with lines title "42",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 43:134 with lines title "43",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 44:135 with lines title "44",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 45:136 with lines title "45",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 46:137 with lines title "46",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 47:138 with lines title "47",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 48:139 with lines title "48",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 49:140 with lines title "49",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 50:141 with lines title "50",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 51:142 with lines title "51",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 52:143 with lines title "52",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 53:144 with lines title "53",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 54:145 with lines title "54",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 55:146 with lines title "55",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 56:147 with lines title "56",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 57:148 with lines title "57",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 58:149 with lines title "58",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 59:150 with lines title "59",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 60:151 with lines title "60",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 61:152 with lines title "61",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 62:153 with lines title "62",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 63:154 with lines title "63",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 64:155 with lines title "64",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 65:156 with lines title "65",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 66:157 with lines title "66",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 67:158 with lines title "67",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 68:159 with lines title "68",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 69:160 with lines title "69",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 70:161 with lines title "70",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 71:162 with lines title "71",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 72:163 with lines title "72",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 73:164 with lines title "73",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 74:165 with lines title "74",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 75:166 with lines title "75",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 76:167 with lines title "76",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 77:168 with lines title "77",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 78:169 with lines title "78",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 79:170 with lines title "79",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 80:171 with lines title "80",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 81:172 with lines title "81",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 82:173 with lines title "82",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 83:174 with lines title "83",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 84:175 with lines title "84",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 85:176 with lines title "85",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 86:177 with lines title "86",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 87:178 with lines title "87",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 88:179 with lines title "88",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 89:180 with lines title "89",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 90:181 with lines title "90",\
"< paste /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_time.dat /export/wolf1768/testoutput/CellModelTests/aslanidi_Purkinje_model_2009/S1S2/outputs_membrane_voltage.dat" using 91:182 with lines title "91"
