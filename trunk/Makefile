INCLUDEPATH = 
MAGICKFLAGS = `Magick++-config --cppflags --cxxflags --ldflags --libs`
C++ = g++

all: circlefinder orientpage

orientpage: orientpage.o hough.o geometricPrimitives.o
	$(C++) -g $(MAGICKFLAGS) orientpage.o hough.o geometricPrimitives.o \
		-o orientpage

circlefinder: circlefinder.o hough.o geometricPrimitives.o
	$(C++) -g $(MAGICKFLAGS) circlefinder.o hough.o geometricPrimitives.o \
		-o circlefinder

orientpage.o: geometricPrimitives.hpp hough.hpp orientpage.cpp
	$(C++) -g -c $(INCLUDEPATH) $(MAGICKFLAGS) -o orientpage.o \
		orientpage.cpp

circlefinder.o: geometricPrimitives.hpp hough.hpp circlefinder.cpp
	$(C++) -g -c $(INCLUDEPATH) $(MAGICKFLAGS) -o circlefinder.o \
		circlefinder.cpp

hough.o: geometricPrimitives.hpp hough.cpp hough.hpp
	$(C++) -g -c $(INCLUDEPATH) $(MAGICKFLAGS) -o hough.o hough.cpp

geometricPrimitives.o: geometricPrimitives.cpp geometricPrimitives.hpp
	$(C++) -g -c $(INCLUDEPATH) $(MAGICKFLAGS) -o geometricPrimitives.o \
		geometricPrimitives.cpp

clean:
	rm *.o 
