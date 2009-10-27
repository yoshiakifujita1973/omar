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

  templateImageTL.modifyImage();
  templateImageBR.modifyImage();

  vector<Circle> templateTopCircles = 
    findCircles(templateImageTL, 2, minRadius, columns/8);

  vector<Circle> templateBottomCircles = 
    findCircles(templateImageBR, 2, minRadius, columns/8);
  
  templateImageTL.fillColor(Magick::Color("none"));
  templateImageTL.strokeColor(Magick::Color("red"));
  templateImageTL.strokeWidth(1);
  templateImageBR.fillColor(Magick::Color("none"));
  templateImageBR.strokeColor(Magick::Color("red"));
  templateImageBR.strokeWidth(1);
  templateImageTL.draw(templateTopCircles[0].toDrawable());
  templateImageTL.draw(templateTopCircles[1].toDrawable());
  templateImageBR.draw(templateBottomCircles[0].toDrawable());
  templateImageBR.draw(templateBottomCircles[1].toDrawable());
  templateImageTL.display();
  templateImageBR.display();

  if(templateTopCircles[0].getCenter().distanceFrom(templateTopCircles[1].getCenter()) > 3){
    cerr << "Top centers too far apart!" << endl;
    return 1;
  }

  if(templateBottomCircles[0].getCenter().distanceFrom(templateBottomCircles[1].getCenter())
     > 3){
    cerr << "Bottom centers too far apart!" << endl;
    return 1;
  }

  rotatedImage.modifyImage();


  Point TTL0 = templateTopCircles[0].getCenter();
  Point TTL1 = templateTopCircles[1].getCenter();
  Point TBR0 = templateBottomCircles[0].getCenter();
  Point TBR1 = templateBottomCircles[1].getCenter();

  Point TTL = Point((TTL0.getX() + TTL1.getX())/2,
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
  rotatedImage.display();

  Magick::Image rotatedImageBR = rotatedImage;
  rotatedImageBR.crop(Magick::Geometry(columns,rows/4, 0,
				       3*rows/4));

  Magick::Image rotatedImageTL = rotatedImage;
  rotatedImageTL.crop(Magick::Geometry(columns, rows/4));

  rotatedImageTL.modifyImage();
  rotatedImageBR.modifyImage();

  vector<Circle> rotatedTopCircles = findCircles(rotatedImageTL, 2, 
						 minRadiusTop-1,
						 maxRadiusTop+1);


  vector<Circle> rotatedBottomCircles = findCircles(rotatedImageBR, 2, 
						 minRadiusBottom-1,
						 maxRadiusBottom+1);

						 
  rotatedImageTL.fillColor(Magick::Color("none"));
  rotatedImageTL.strokeColor(Magick::Color("red"));
  rotatedImageTL.strokeWidth(1);
  rotatedImageTL.draw(rotatedTopCircles[0].toDrawable());
  rotatedImageTL.draw(rotatedTopCircles[1].toDrawable());
  rotatedImageBR.fillColor(Magick::Color("none"));
  rotatedImageBR.strokeColor(Magick::Color("red"));
  rotatedImageBR.strokeWidth(1);
  rotatedImageBR.draw(rotatedBottomCircles[0].toDrawable());
  rotatedImageBR.draw(rotatedBottomCircles[1].toDrawable());
  rotatedImageTL.display();
  rotatedImageBR.display();

  Point RTL0 = rotatedTopCircles[0].getCenter();
  Point RTL1 = rotatedTopCircles[1].getCenter();
  Point RBR0 = rotatedBottomCircles[0].getCenter();
  Point RBR1 = rotatedBottomCircles[1].getCenter();

  Point RTL = Point((RTL0.getX() + RTL1.getX())/2,
		    (RTL0.getY() + RTL1.getY())/2);
  Point RBR = Point((RBR0.getX() + RBR1.getX())/2,
		    (RBR0.getY() + RBR1.getY())/2 + 3*rows/4);

  
  double angleTemplate = atan((TTL.getY() - TBR.getY()) * 1.0 / 
			      (TTL.getX() - TBR.getX() ));

  double angleRotated = atan((RTL.getY() - RBR.getY()) * 1.0 / 
			      (RTL.getX() - RBR.getX()));

  rotatedTopCircles[0].print();
  rotatedTopCircles[1].print();
  rotatedBottomCircles[0].print();
  rotatedBottomCircles[1].print();

  
  cout << "arctangent" << endl;
  cout << "template angle: " << 180 * angleTemplate / M_PI << endl;
  cout << "rotated angle: " << 180 * angleRotated / M_PI << endl;

  angleTemplate = asin(-(TTL.getY() - TBR.getY()) / TTL.distanceFrom(TBR));
  
  angleRotated = asin(-(RTL.getY() - RBR.getY()) / RTL.distanceFrom(RBR));
  
  cout << "arcsine" << endl;
  cout << "template angle: " << angleTemplate * 180 / M_PI << endl;
  cout << "rotated angle: " << angleRotated  * 180 / M_PI << endl;

  TTL.print();
  TBR.print();
  RTL.print();
  RBR.print();
}
