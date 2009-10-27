#include<stdio.h>
#include<vector>
#include<queue>
#include<unistd.h>
#include<math.h>
//#include<stdlib>
#include<iostream>
#include<Magick++.h>
#include"geometricPrimitives.hpp"

#define MIN(x,y) (x)>(y)?(y):(x)
#define MAX(x,y) (x)<(y)?(y):(x)


#define MARK_THRESHOLD 0.85
#define AT_WIDTH 20
#define AT_HEIGHT 20
#define AT_OFFSET 57000
#define MARK_MAX_MEASURE 30

using namespace std;

Circle cleararea(int i, int j, vector<vector< bool > > *markedPixels,
		 Magick::Image image){
  queue<Point> pointQueue;
  vector<Point> pointList;
  Magick::Color black = Magick::Color("black");
  int columns = image.columns();
  int rows = image.rows();
  int sumX = 0;
  int sumY = 0;
  int numberPixels = 0;
  int maxX = 0;
  int minX = columns;
  int maxY = 0;
  int minY = rows;

  pointQueue.push(Point(i,j));
  pointList.push_back(Point(i,j));
  (*markedPixels)[i][j] = true;
  
  while(pointQueue.size() > 0){
    Point pointToCheck = pointQueue.front();
    pointQueue.pop();
    int x = pointToCheck.getX();
    int y = pointToCheck.getY();
    sumX += x;
    sumY += y;
    numberPixels++;
    maxX = MAX(x,maxX);
    maxY = MAX(y,maxY);
    minX = MIN(x,minX);
    minY = MIN(y,minY);
    for(int k = x-1; k <= x+1; k++){
      if((*markedPixels)[k].size() <= rows) 
	(*markedPixels)[k].resize(rows+1);
      for(int l = y-1; l <= y+1; l++){
	if(k > 0 && l > 0 
	   && k < columns && l < rows 
	   && (*markedPixels)[k][l] == false
	   && image.pixelColor(k,l) == black){
	  (*markedPixels)[k][l] = true;
	  pointQueue.push(Point(k,l));
	  pointList.push_back(Point(k,l));
	}
      }
    }
  }
  int avgX = sumX * 1.0 / numberPixels + 0.5;
  int avgY = sumY * 1.0 / numberPixels + 0.5;
  int avgR = (maxX - minX + maxY - minY) / 4.0 + 0.5;
  Point foundPoint = Point(avgX, avgY);
  Circle foundCircle = Circle(foundPoint, avgR);
  int containedPoints = 0;
  for(vector<Point>::iterator it = pointList.begin(); 
      it != pointList.end(); it++){
    if(foundCircle.contains(*it))
      containedPoints++;
  }
  if(foundCircle.getArea() * MARK_THRESHOLD <= containedPoints){
    return foundCircle;
  }
  Point origin = Point(0,0);
  return Circle(origin,0);
}


vector<Circle> findmarks(Magick::Image image){
  int columns = image.columns();
  int rows = image.rows();
  vector<vector< bool > > markedPixels(columns);
  Magick::Color black = Magick::Color("black");
  Magick::Image thresholdImage = image;
  thresholdImage.type(Magick::GrayscaleType);
  thresholdImage.threshold(AT_OFFSET);
  thresholdImage.display();
  vector<Circle> marks;
  
  for(int i = 0; i < columns; i++){
    if(markedPixels.size() <= i) markedPixels.resize(i+1);
    for(int j = 0; j < rows; j++){
      if(thresholdImage.pixelColor(i,j) == black){
	if(markedPixels[i].size() <= j){
	  markedPixels[i].resize(j+1);
	}
	if(markedPixels[i][j] == false){
	  Circle newMark = cleararea(i,j, &markedPixels, thresholdImage);
	  if(newMark.getRadius() > 0)
	    marks.push_back(newMark);
	}
      }
    }
  }

  return marks;
}
