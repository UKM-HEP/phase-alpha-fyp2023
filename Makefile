all : rc
rc : rc_analyser

rc_analyser : rc_analyser.cxx
	g++ `root-config --cflags --libs` -o rc_analyser rc_analyser.cxx -Wall -g 

clean:
	rm -f rc_analyser
