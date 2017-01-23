10 print chr$(147):rem ctrl port test pgm
20 print "joystick port 1:"peek(56321)
30 print "joystick port 2:"peek(56320)
40 print: poke56333,127:poke56320,64
50 print "port 1 pot x:"peek(54298)
60 print "port 1 pot y:"peek(54297)
70 poke56320,128
80 print "port 2 pot x:"peek(54298)
90 print "port 2 pot y:"peek(54297)
100 poke56333,129
110 print chr$(19)
120 goto20
