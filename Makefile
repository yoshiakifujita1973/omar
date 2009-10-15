INCLUDEPATH = -I/usr/share/lib
MAGICKFLAGS = `Magick++-config --cppflags --cxxflags --ldflags --libs`

geometricPrimatives.o: geometricPrimatives.cpp geometricPrimatives.hpp
	g++ -g -c $(INCLUDEPATH) $(MAGICKFLAGS) -o geometricPrimatives.o \
		geometricPrimatives.cpp

hough.o: geometricPrimatives.o hough.cpp hough.hpp
	g++ -g -c $(INCLUDEPATH) $(MAGICKFLAGS) -o hough.o hough.cpp \
		geometricPrimatives.o
clean:
	rm *.o 