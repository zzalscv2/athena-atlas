

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs)

SOURCES = hcg.cxx addnode.cxx

INSTALLDIR=$(WorkDir_DIR)/bin

# install : hcg
# 	cp hcg $(HOME)/bin 

hcg : $(SOURCES) 
	g++ $(ROOTCFLAGS) -o $@ $(SOURCES) $(ROOTLIBS)

archive :  $(SOURCES) node.h addnode.h simpletimer.h spacer.h makefile README
	tar -czf hcg.tgz $?

install : hcg
	cp hcg $(HOME)/bin

ginstall : install
	cp hcg $(INSTALLDIR)
