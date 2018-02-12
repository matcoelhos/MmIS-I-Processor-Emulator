CC = g++
FILES = processor.cpp
OPTIONS = -std=c++11 -O2 -Ofast

#main section
main: $(FILES)
	$(CC) $(OPTIONS) $(FILES) -o Processor

clear:
	rm -f Processor
