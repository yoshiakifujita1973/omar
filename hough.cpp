#include<vector>
#include<queue>
#include<list>
#include<Magick++.h>
#include<iostream>
#include<math.h>
#include"geometricPrimatives.hpp"
#include"hough.hpp"

#define MIN(x, y) ((x)>(y)?(y):(x))
#define MAX(x, y) ((x)>(y)?(x):(y))

using namespace std;
using namespace Magick;

vector<Point> getPoints( Image image ){
  /* Set the image type to TrueColor DirectClass representation */
  image.type(TrueColorType);
  /* Ensure we're dealing with an updated copy of the image */
  image.modifyImage();
  vector<Point> retval;
  /* get a pixel view of the image */
  Pixels view(image);

  /* find the height and width of the picture */
  unsigned int height = image.rows();
  unsigned int width = image.columns();
  
  /* put the height and width as the first point on our stack */
  //retval.push_back(Point(width,height)); 

  Color white("white");

  PixelPacket *pixels = view.get(0,0,width,height);
  for (unsigned int row = 0; row < height; row++){ 
    for (unsigned int column = 0; column < width; column++){
      if (*pixels == white){
	retval.push_back(Point(row, column));
      }
    }
  }
  
  return retval;
}

void vote(int a, int b, int r, int maxA, 
	  int maxB, int maxR,
	  vector<vector<vector< int > > > votes){
  for(int i = MAX(0, a-1); i <= MIN(maxA, a+1); i++){
    for(int j = MAX(0, b-1); j <= MIN(maxB, b+1); j++){
      for(int k = MAX(0, r-1); k <= MIN(maxR, r+1); k++){
	if (votes.size() < i) votes.resize(i);
	if (votes[i].size() < j) votes[i].resize(j);
	if (votes[i][j].size() < k) {
	  votes[i][j].resize(k);
	  votes[i][j][k] = 0;
	}
	votes[i][j][k]++;
      }
    }
  }
  votes[a][b][r]++;
}

vector<vector<vector< int > > >  electCircles(vector<Point> points,
					      int maxRadius, 
					      Point lowerRightCorner) {
  vector<vector<vector< int > > > votes;
  int maxA = lowerRightCorner.getX();
  int maxB = lowerRightCorner.getY();
  for(vector<Point>::iterator p = points.begin(); p != points.end(); ++p){
    int i = p->getX();
    int j = p->getY();
    for(int r = 0; r <= maxRadius; r++){
      for(int a = MAX(0, i - r); a <= MIN(maxA, i + r); a++){
	int b = sqrt((r^2) - ((i - a)^2));
	if (b+j < maxB) vote(a, b+j, r, maxA, maxB, maxRadius, votes);
	if (b > 0 && b - j > 0) vote(a, j-b, r, maxA, maxB, maxRadius, votes);
      }
    }
  }
  return votes;
}

double circleValue(Circle circle, int votes){
  return votes / circle.getArea();
}

Circle clearArea(int x, int y, int r, vector<vector<vector< bool > > > *marked,
		 vector<vector<vector< int > > > votes, double threshold){
  queue<Circle> circlesToCheck;
  Point tempPoint = Point(x, y);
  Circle returnCircle = Circle(tempPoint, r);

  (*marked)[x][y][r] = true;
  circlesToCheck.push(returnCircle);
  while(!circlesToCheck.empty()){
    Circle currentCircle = circlesToCheck.front();
    circlesToCheck.pop();
    int a = currentCircle.getCenter().getX();
    int b = currentCircle.getCenter().getY();
    int r = currentCircle.getRadius();
    int maxA = votes.size();
    int maxB = votes[a].size();
    int maxR = votes[a][b].size();
    int maxCircleValue = 0;
    for(int i = MAX(0, a-1); i <= MIN(maxA, a+1); i++){
      for(int j = MAX(0, b-1); j <= MIN(maxB, b+1); j++){
	for(int k = MAX(0, r-1); k <= MIN(maxR, r+1); k++){
	  Point tempPoint = Point(i, j);
	  Circle newCircle = Circle(tempPoint, k);
	  double weight = circleValue(newCircle, votes[i][j][k]);
	  if((*marked)[i][j][k] == false && weight > threshold){
	    (*marked)[i][j][k] = true;
	    circlesToCheck.push(newCircle);
	    if(weight > maxCircleValue){
	      maxCircleValue = weight;
	      returnCircle = newCircle;
	    }
	  }
	}
      }
    }
  }
  return returnCircle;
}

vector<Circle> chooseWinners(const vector<vector<vector< int > > > votes,
			     double threshold){
  vector<vector<vector< bool> > > marked;
  vector<Circle> circleList;

  marked.resize(votes.size());
  for(int i = 0; i < votes.size(); i++){
    marked[i].resize(votes[i].size());
    for(int j = 0; j < votes[i].size(); j++){
      marked[i][j].resize(votes[i][j].size());
      for(int r = 0; r < votes[i][j].size(); r++){
	marked[i][j][r] = false;
      }
    }
  }

  for(int i = 0; i < votes.size(); i++){
    for(int j = 0; j < votes[i].size(); j++){
      for(int r = 0; r < votes[i][j].size(); r++){
	Point circleCenter = Point(i, j);
	Circle circle = Circle(circleCenter, r);
	int value = circleValue(circle, votes[i][j][r]);
	if(value > threshold){
	  marked[i][j][r] = 1;
	  circleList.push_back(clearArea(i, j, r, &marked, votes, threshold));
	}
      }
    }
  }
  return circleList;
}
