#include<stdio.h>
#include<vector>
#include<unistd.h>
#include<math.h>
//#include<stdlib>
#include<iostream>
#include<Magick++.h>
#include"geometricPrimitives.hpp"
#include"hough.hpp"

#define MIN(x, y) ((x)>(y)?(y):(x))
#define MAX(x, y) ((x)>(y)?(x):(y))

using namespace std;
//using namespace Magick;

//extern ostream cerr;
//extern ostream cout;


int main(int argc, char *argv[]){
  int minRadius = 10;
  int maxRadius = 0;
  int numberOfCircles = 0;
  char *templatePage = NULL;
  char *rotatedPage = NULL;
  int c;

  opterr = 0;

  while((c = getopt(argc, argv, "t:r:")) != EOF){
    switch (c){
    case 't':
      templatePage = optarg;
      break;
    case 'r':
      rotatedPage = optarg;
      break;
    case '?':
      if (optopt == 't' || optopt == 'r')
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

  if(templatePage == NULL || rotatedPage == NULL){
    cerr << "Usage: " << endl;
    cerr << argv[0] << " -t <template page> -r <rotated page>" << endl;
    return 1;
  }

  Magick::Image templateImage;
  try {
    templateImage.read(templatePage);
  } 
  catch(Magick::Exception &error_) {
    cerr << "Exception caught on file \"" << templatePage << "\". Exception: " 
	 << error_.what() << endl;
  } 

  Magick::Image rotatedImage;
  try {
    rotatedImage.read(rotatedPage);
  } 
  catch(Magick::Exception &error_) {
    cerr << "Exception caught on file \"" << rotatedPage << "\". Exception: " 
	 << error_.what() << endl;
  } 

  templateImage.edge(1);
  rotatedImage.edge(1);

  templateImage.monochrome();
  rotatedImage.monochrome();

  Magick::Image templateImageTL = templateImage;
  int columns = templateImage.columns();
  int rows = templateImage.rows();
  templateImageTL.crop(Magick::Geometry(columns/4,columns/4));
  Magick::Image templateImageBR = templateImage;
  templateImageBR.crop(Magick::Geometry(columns/4,columns/4,3*columns/4,
					rows - columns/4));

  vector<Circle> templateTopCircles = 
    findCircles(templateImageTL, 2, minRadius, columns/8);

  vector<Circle> templateBottomCircles = 
    findCircles(templateImageBR, 2, minRadius, columns/8);
  
  if(templateTopCircles[0].getCenter().distanceFrom(templateTopCircles[1].getCenter()) > 3){
    cerr << "Top centers too far apart!" << endl;
    return 1;
  }

  if(templateBottomCircles[0].getCenter().distanceFrom(templateBottomCircles[1].getCenter())
     > 3){
    cerr << "Bottom centers too far apart!" << endl;
    return 1;
  }

  Point TTL0 = templateTopCircles[0].getCenter();
  Point TTL1 = templateTopCircles[1].getCenter();
  Point TBR0 = templateBottomCircles[0].getCenter();
  Point TBR1 = templateBottomCircles[1].getCenter();

  Point TTL = Point((TTL0.getX() + TTL1.getX()/2),
		    (TTL0.getY() + TTL1.getY())/2);
  Point TBR = Point((TBR0.getX() + TBR1.getX())/2 + 3*columns/4,
		    (TBR0.getY() + TBR1.getY())/2 + rows - columns/4); 

  columns = rotatedImage.columns();
  rows = rotatedImage.rows();

  int minRadiusTop = MIN(templateTopCircles[0].getRadius(),
			 templateTopCircles[1].getRadius());

  int maxRadiusTop = MAX(templateTopCircles[0].getRadius(),
			 templateTopCircles[1].getRadius());

  int minRadiusBottom = MIN(templateBottomCircles[0].getRadius(),
			    templateBottomCircles[1].getRadius());

  int maxRadiusBottom = MAX(templateBottomCircles[0].getRadius(),
			    templateBottomCircles[1].getRadius());

  Magick::Image rotatedImageTL = rotatedImage;
  rotatedImageTL.crop(Magick::Geometry(columns,columns/4));
  Magick::Image rotatedImageBR = rotatedImage;
  rotatedImageBR.crop(Magick::Geometry(columns,columns/4, 0,
				       rows - columns/4));

  vector<Circle> rotatedTopCircles = findCircles(rotatedImageTL, 2, 
						 minRadiusTop-2,
						 maxRadiusTop+2);

  vector<Circle> rotatedBottomCircles = findCircles(rotatedImageBR, 2, 
						 minRadiusBottom-2,
						 maxRadiusBottom+2);
						 
  Point RTL0 = rotatedTopCircles[0].getCenter();
  Point RTL1 = rotatedTopCircles[1].getCenter();
  Point RBR0 = rotatedBottomCircles[0].getCenter();
  Point RBR1 = rotatedBottomCircles[1].getCenter();

  Point RTL = Point((RTL0.getX() + RTL1.getX()/2),
		    (RTL0.getY() + RTL1.getY())/2);
  cout << columns << " " << rows << endl;
  Point RBR = Point((RBR0.getX() + RBR1.getX())/2,
		    (RBR0.getY() + RBR1.getY())/2 + rows - columns/4);

  
  double angleTemplate = atan((TTL.getY() - TBR.getY()) * 1.0 / 
			      (TTL.getX() - TBR.getX()));

  double angleRotated = atan((RTL.getY() - RBR.getY()) * 1.0 / 
			      (RTL.getX() - RBR.getX()));

  
  RTL0.print();
  RTL1.print();
  RBR0.print();
  RBR1.print();
  RBR.print();
  cout << "arctangent" << endl;
  cout << "template angle: " << 180 * angleTemplate / M_PI << endl;
  cout << "rotated angle: " << 180 * angleRotated / M_PI << endl;

  angleTemplate = asin(-(TTL.getY() - TBR.getY()) / TTL.distanceFrom(TBR));
  
  angleRotated = asin(-(RTL.getY() - RBR.getY()) / RTL.distanceFrom(RBR));
  
  cout << "arcsine" << endl;
  cout << "template angle: " << angleTemplate * 180 / M_PI << endl;
  cout << "rotated angle: " << angleRotated  * 180 / M_PI << endl;
}
