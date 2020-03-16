CXX=g++
CXXFLAGS=-Wall -O3
BIN=lcdfm
INSTDIR=/usr/local/lib/lcdfm
UTILS=lcdfm.sh lcdfm.conf aacplayer.sh m3uplayer.sh urlplayer.sh vlcplayer.sh modify_vlc.sh

all : $(BIN)

clean : 
	rm -f $(BIN)

install : $(BIN)
	mkdir -p $(INSTDIR)
	cp $(BIN) $(INSTDIR)
	cp $(UTILS) $(INSTDIR)

lcdfm : lcdfm.cc
	$(CXX) $(CXXFLAGS) -o $@ $< -lncursesw
