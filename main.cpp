#include<stdio.h>
#include<vector>
#include<unistd.h>
//#include<stdlib>
#include<iostream>
#include<Magick++.h>
#include"geometricPrimitives.hpp"
#include"hough.hpp"

using namespace std;
//using namespace Magick;

//extern ostream cerr;
//extern ostream cout;


int main(int argc, char *argv[]){
  int minRadius = 0;
  int maxRadius = 0;
  int numberOfCircles = 0;
  char *filename = NULL;
  int c;

  opterr = 0;

  while((c = getopt(argc, argv, "n:x:c:")) != EOF){
    switch (c){
    case 'n':
      minRadius = atoi(optarg);
      break;
    case 'x':
      maxRadius = atoi(optarg);
      break;
    case 'c':
      numberOfCircles = atoi(optarg);
      break;
    case '?':
      if (optopt == 'n' || optopt == 'x' || optopt == 'c')
	cerr << "Option -" << optopt << " requires an argument!\n";
      else if (isprint(optopt))
	cerr << "Unknown option -" << char(optopt) << endl;
      else 
	cerr << "Unknown option character" << endl;
      return 1;
      
      break;
    default:
      abort();
    }
  }

  if(maxRadius == 0 || minRadius == 0 || numberOfCircles == 0 
     || optind == argc){
    cerr << "Usage: " << endl;
    cerr << argv[0] << " -n <minimum radius> -x <maximum radius> -c <number of circles> <picture file>" << endl;
    return 1;
  }
  
  filename = argv[optind];
  Magick::Image image;
  try {
    image.read(filename);
  } 
  catch(Magick::Exception &error_) {
    cout << "Caught on file \"" << filename << "\". Exception: " 
	 << error_.what() << endl;
  } 

}
