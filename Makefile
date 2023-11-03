all : rc wfviewer_rc rc_textanalyser
rc : rc_analyser

rc_analyser : rc_analyser.cxx
	g++ `root-config --cflags --libs` -o rc_analyser rc_analyser.cxx -Wall -g 

wfviewer_rc: wfviewer_rc.cxx
	g++ `root-config --cflags --libs` -o wfviewer_rc wfviewer_rc.cxx -Wall -g

rc_textanalyser : rc_textanalyser.cxx
	g++ `root-config --cflags --libs` -o rc_textanalyser rc_textanalyser.cxx -Wall -g 

clean:
	rm -f rc_analyser wfviewer_rc rc_textanalyser
