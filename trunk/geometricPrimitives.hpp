
#ifndef GEOMETRICPRIMITIVES_HPP
#define GEOMETRICPRIMITIVES_HPP

#include<Magick++.h>

#define GP_LINE_CARTESIAN 0x1
#define GP_LINE_POLAR 0x2

class Line;
class Circle;

class Point{
public:
  Point();
  Point(int, int);

  int getX();
  int getY();
  double getR();
  double getTheta();
  double distanceFrom(Point);

  bool isOnLine(Line);
  bool isOnEdge(Circle);

  void print();

  Point operator=(const Point &);
  bool operator==(const Point &);

private:
  int xCoordinate, yCoordinate;
  double r, theta;
};

class Circle{
public:
  Circle(Point &, int);

  int getRadius();
  int getDiameter();
  double getCircumference();
  Point getCenter();
  double getArea();
  bool contains(Point);
  
  Magick::DrawableCircle toDrawable();

  void print();

  bool operator==(const Circle &);

private:
  int radius;
  Point center;
};

class Line{
public:
  Line(double, double, int);

  double getSlope();
  double getYIntercept();
  double getTheta();
  double getR();

  bool pointOnLine(Point);
  bool bisects(Circle);
  bool tangentTo(Circle);

  bool operator==(const Line &);
  

private:
  double theta;
  double r;
  double sinTheta;
  double cosTheta;
  double m;
  double b;
};


#endif
