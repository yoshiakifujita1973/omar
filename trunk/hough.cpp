#include<vector>
#include<queue>
#include<list>
#include<map>
#include<algorithm>
#include<Magick++.h>
#include<iostream>
#include<math.h>
#include"geometricPrimitives.hpp"
#include"hough.hpp"

#define MIN(x, y) ((x)>(y)?(y):(x))
#define MAX(x, y) ((x)>(y)?(x):(y))
#define MAXVALUE 15
#define THRESHOLDSTEP 0.1

using namespace std;
using namespace Magick;

/* get the "hot" points on an image.  return them as a vector of Points.
 * right now we look for white pixels, which we hopefully got 
 * after doing an edge detect and thresholding on the image
 */
vector<Point> getPoints( Image image ){
  /* Set the image type to TrueColor DirectClass representation */
  image.type(BilevelType);
  /* Ensure we're dealing with an updated copy of the image */
  image.modifyImage();
  vector<Point> retval;
  /* get a pixel view of the image */

  /* find the height and width of the picture */
  unsigned int height = image.rows();
  unsigned int width = image.columns();
  
  /* put the height and width as the first point on our stack */
  //retval.push_back(Point(width,height)); 

  /* we're looking for white */
  Color white("white");
  /* put all the white pixels on the vectore of points */
  for (unsigned int row = 0; row < height; row++){ 
    for (unsigned int column = 0; column < width; column++){
      /* White pixels are set to true */
      if (image.pixelColor(column, row) == white){
	retval.push_back(Point(column, row));
      }
    }
  }
  
  return retval;
}


/* vote for the circle given by a, b, and r.
 * vote four times for the circle a, b, r, twice for
 * its direct neighbors and once for each of its diagonal neighbors
 * because we're using ints in a real word 
 */
void vote(int a, int b, int r, int maxA, 
	  int maxB, int maxR,
	  vector<vector<vector< int > > > *votes){
  for(int i = MAX(0, a-1); i <= MIN(maxA, a+1); i++){
    for(int j = MAX(0, b-1); j <= MIN(maxB, b+1); j++){
      for(int k = MAX(0, r-1); k <= MIN(maxR, r+1); k++){
	if (votes->size() <= i){
	  votes->resize(i+1);
	}
	if ((*votes)[i].size() <= j){
	  (*votes)[i].resize(j+1);
	}
	if ((*votes)[i][j].size() <= k) {
	  (*votes)[i][j].resize(k+1);
	  (*votes)[i][j][k] = 0;
	}
	(*votes)[i][j][k]++;
	if((i == a) + (j == b) + (k == r) > 1) (*votes)[i][j][k]++;
        if(i == a && j == b && k == r) (*votes)[i][j][k] += 2;
      }
    }
  }
}

/* take the "hot" points on the image, and vote for their possible circles
 * return the 3d array of the votes (x and y position, and radius) 
 */
vector<vector<vector< int > > >  electCircles(vector<Point> *points,
					      int minRadius, int maxRadius, 
					      Point lowerRightCorner) {
  vector<vector<vector< int > > > votes;
  int maxA = lowerRightCorner.getX();
  int maxB = lowerRightCorner.getY();
  vector<vector<int> > coneSlice;
  for(int r = minRadius; r <= maxRadius; r++){
    if(coneSlice.size() <= r) coneSlice.resize(r+1);
    int b = r;
    int r2 = r*r;
     for(int a = 0; a < b; a++){
      if(coneSlice[r].size() <= a) coneSlice[r].resize(a+1);
      b = (sqrt(r2 - a*a) + 0.5);
      coneSlice[r][a] = b;
    }
  }
  /* iterate through the points */
  for(vector<Point>::iterator p = points->begin(); p != points->end(); ++p){
    int i = p->getX();
    int j = p->getY();
    /* vote for expanding circles around that point */
    for(int r = minRadius; r <= maxRadius; r++){
      /* vote for the corners of the circles */
      vote(i + r, j, r, maxA, maxB, maxRadius, &votes);
      vote(i - r, j, r, maxA, maxB, maxRadius, &votes);
      vote(i, j + r, r, maxA, maxB, maxRadius, &votes);
      vote(i, j - r, r, maxA, maxB, maxRadius, &votes);
      
      /* get pairs of values a, b that are on a circle of radius r */
      for(int a = 1; a < coneSlice[r].size(); a++){
	int b = coneSlice[r][a];
	/* vote for the eight points on this circle that are offset
	   from the center by a combination of a and b -- the reflections
	   on the axes centered at the circle's center */
	vote(i + a, j + b, r, maxA, maxB, maxRadius, &votes);
	vote(i - a, j + b, r, maxA, maxB, maxRadius, &votes);
	vote(i + a, j - b, r, maxA, maxB, maxRadius, &votes);
	vote(i - a, j - b, r, maxA, maxB, maxRadius, &votes);
	vote(i + b, j + a, r, maxA, maxB, maxRadius, &votes);
	vote(i - b, j + a, r, maxA, maxB, maxRadius, &votes);
	vote(i + b, j - a, r, maxA, maxB, maxRadius, &votes);
	vote(i - b, j - a, r, maxA, maxB, maxRadius, &votes);
      }
    }
  }
  return votes;
}

/* this function says how we should weight the votes.  
 * we use votes / circumference because a perfect circle would be
 * 1 if every point on the circle cast a vote for that circle 
 */
double circleValue(Circle circle, int votes){
  return votes / circle.getCircumference();
}

/* take a seed point in the vote space, and find all its neighbors that are
 * above the threshold, assuming they all belong to the same circle
 * return the circle that got the highest number of votes.
 * ISSUE: should deal with ties somehow!
 */

pair<double, Circle> clearArea(int x, int y, int r, 
			       vector<vector<vector< bool > > > *marked,
			       vector<vector<vector< int > > > *votes, 
			       double threshold){
  /* we need a queue to keep track of the circles we're going to check */
  queue<Circle> circlesToCheck;
  Point tempPoint = Point(x, y);
  /* make a circle out of x, y, and r */
  Circle returnCircle = Circle(tempPoint, r);

  /* record that we've looked at the circle x, y, r */
  (*marked)[x][y][r] = true;
  /* and put it on the queue */
  circlesToCheck.push(returnCircle);

  /* the first circle has the best value we've seen so far, so record it! */
  double maxCircleValue = circleValue(returnCircle, (*votes)[x][y][r]);

  /* while there are still circles left to check */
  while(!circlesToCheck.empty()){
    /* pull one off the queue */
    Circle currentCircle = circlesToCheck.front();
    circlesToCheck.pop();
    /* get its information */ 
    int a = currentCircle.getCenter().getX();
    int b = currentCircle.getCenter().getY();
    int r = currentCircle.getRadius();
    /* get the boundries for its position and radius! */
    int maxA = votes->size();
    int maxB = (*votes)[a].size();
    int maxR = (*votes)[a][b].size();
    /* go through this circle's neighbors in the vote space */
    for(int i = MAX(0, a-1); i <= MIN(maxA, a+1); i++){
      for(int j = MAX(0, b-1); j <= MIN(maxB, b+1); j++){
	for(int k = MAX(0, r-1); k <= MIN(maxR, r+1); k++){
	  /* get their relavant data */
	  Point tempPoint = Point(i, j);
	  Circle newCircle = Circle(tempPoint, k);
	  double weight = circleValue(newCircle, (*votes)[i][j][k]);
	  /* and mark them and queue them up if they're unmarked and
	     above the threshold */
	  if((*marked)[i][j][k] == false && weight > threshold){
	    (*marked)[i][j][k] = true;
	    circlesToCheck.push(newCircle);
	    if(weight > maxCircleValue){
	      /* if they're the circle with the most votes so far,
		 make them the current "winner" */
	      maxCircleValue = weight;
	      returnCircle = newCircle;
	    }
	  }
	}
      }
    }
  }
  return pair<double, Circle>(maxCircleValue, returnCircle);
}

/* get a list of the circles which have a heuristic value above threshold
 * clusters of contiguous circles in the votespace which are all above
 * the threshold will be considered one circle.  We will use the one with
 * the highest heuristic value.
 */
multimap<double,Circle> chooseWinners(vector<vector<vector< int > > > *votes,
			     double threshold){
  vector<vector<vector< bool> > > marked;
  multimap<double, Circle> circleList;

  /* clear the circles we've seen */
  marked.resize(votes->size());
  for(int i = 0; i < votes->size(); i++){
    marked[i].resize((*votes)[i].size());
    for(int j = 0; j < (*votes)[i].size(); j++){
      marked[i][j].resize((*votes)[i][j].size());
      for(int r = 0; r < (*votes)[i][j].size(); r++){
	marked[i][j][r] = false;
      }
    }
  }

  /* iterate through the votespace */
  for(int i = 0; i < votes->size(); i++){
    for(int j = 0; j < (*votes)[i].size(); j++){
      for(int r = 0; r < (*votes)[i][j].size(); r++){
	Point circleCenter = Point(i, j);
	Circle circle = Circle(circleCenter, r);
	double value = circleValue(circle, (*votes)[i][j][r]);
	/* if we find a circle with a heursitic above the threshold 
	   that hasn't already been counted in a cluster then find the cluster
	   and add the circle with the best heuristic in the cluster to
	   our list */
	if(value > threshold && marked[i][j][r] == false){
	  marked[i][j][r] = true;
	  pair<double, Circle> pairToAdd 
	    = clearArea(i, j, r, &marked, votes, threshold);
	  circleList.insert(pairToAdd);
	}
      }
    }
  }
  return circleList;
}

/* the big event, the thing people are supposed to call from outside
 * takes an image with its edges already found and thresholded, the 
 * number of circles to find, their minimum and maximum radii, and 
 * returns a vector with that many circles in it.
 */
vector<Circle> findCircles(Image image, int numberOfCircles,
			   int minRadius, int maxRadius){

  /* collect the points in the image (white ones!) */
  vector<Point> points = getPoints(image);
  /* get the lower right corner, so we know how high to go */
  Point lowerRightCorner = Point(image.columns(), image.rows());

  /* hold the elections for circles */
  vector<vector<vector< int > > > votes 
    = electCircles(&points, minRadius, maxRadius, lowerRightCorner);
  multimap<double, Circle> circleMap;
  /* chose the winners  keep declining the threshold until we get 
     enough circles */
  double threshold;
  for(threshold = MAXVALUE; 
      circleMap.size() < numberOfCircles && threshold > 0; 
      threshold -= THRESHOLDSTEP){
    circleMap = chooseWinners(&votes, threshold);
  }

  vector<Circle> circlesToReturn;

  /* if we got too many circles, cut out the ones with the lowest 
   * values until we have the number requested 
   */
  for(multimap<double, Circle>::reverse_iterator it = circleMap.rbegin();
      it != circleMap.rend() && circlesToReturn.size() < numberOfCircles;
      it++){
    circlesToReturn.push_back(it->second);
  }

  return circlesToReturn;
}
