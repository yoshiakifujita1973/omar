INCLUDEPATH = -I/usr/share/lib
MAGICKFLAGS = `Magick++-config --cppflags --cxxflags --ldflags --libs`

hough.o: geometricPrimitives.o hough.cpp hough.hpp
	g++ -g -c $(INCLUDEPATH) $(MAGICKFLAGS) -o hough.o hough.cpp \
		geometricPrimitives.o

geometricPrimitives.o: geometricPrimitives.cpp geometricPrimitives.hpp
	g++ -g -c $(INCLUDEPATH) $(MAGICKFLAGS) -o geometricPrimitives.o \
		geometricPrimitives.cpp

clean:
	rm *.o 
