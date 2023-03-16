CC=g++
CFLAGS= -std=c++11 -Wall -Wextra
#-O0

all: build

build: main.cc vm_constants.h parser.h codeWriter.h 
	$(CC) $(CFLAGS) main.cc -o VMTranslator 

submit: main.cc vm_constants.h parser.h codeWriter.h lang.txt
	zip -R project08.zip main.cc vm_constants.h parser.h codeWriter.h Makefile lang.txt 

clean:
	rm -r VMTranslator *.dSYM

