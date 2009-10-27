#include<stdio.h>
#include<vector>
#include<unistd.h>
#include<math.h>
//#include<stdlib>
#include<iostream>
#include<Magick++.h>
#include"geometricPrimitives.hpp"
#include"findmarks.hpp" 


int main(int argc, char *argv[]){
  int minRadius = 0;
  int maxRadius = 0;
  int numberOfCircles = 0;
  char *filename = NULL;
  int c;

  if(argc < 1){
    cerr << "Usage: " << endl;
    cerr << argv[0] << " <markfile>" << endl;
    return 1;
  }

  filename = argv[1];

  Magick::Image image;
  
  try {
    image.read(filename);
  }
  catch(Magick::Exception &error_){
    cerr << "Exception caught on file \"" << filename << "\". Exception: " 
         << error_.what() << endl;
    return 1;
  }

  vector<Circle> marks = findmarks(image);

  image.fillColor(Magick::Color("none"));
  image.strokeColor(Magick::Color("red"));
  image.strokeWidth(1);
  for(vector<Circle>::iterator it = marks.begin(); it != marks.end(); it++){
    image.draw(it->toDrawable());
  }

  image.display();

  return 0;
}
