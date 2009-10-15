#include<vector>
#include<unistd.h>
//#include<stdlib>
#include<ostream>
#include<Magick++.h>
#include"geometricPrimitives.hpp"
#include"hough.hpp"

using namespace std;
//using namespace Magick;

extern ostream cerr;
extern ostream cout;


int main(int argc, char *argv[]){
  int minRadius = 0;
  int maxRadius = 0;
  int numberOfCircles = 0;
  char *filename = NULL;
  int c;

  opterr = 0;

  while((c = getopt(argc, argv, "nxc:")) != -1){
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
	cerr << "Option -"<< optopt << " requires an argument!\n";
      else if (isprint(optopt))
	cerr << "Unknown option -" << optopt << endl;
      else 
	cerr << "Unknown option character" << endl;
      return 1;
      
      break;
    default:
      abort();
    }
  }
  for (int index = optind; index < argc; index++){
    filename = argv[index];
    Magick::Image image;
    try {
      image.read(filename);
    } 
    catch(Magick::Exception &error_) {
      cout << "Caught on file \"" << filename << "\". Exception: " 
	   << error_.what() << endl;
    } 
  }

}
