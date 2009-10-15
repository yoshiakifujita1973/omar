#include<vector>
#include<queue>
#include<list>
#include<Magick++.h>
#include<iostream>
#include<math.h>
#include"geometricPrimitives.hpp"
#include"hough.hpp"

#define MIN(x, y) ((x)>(y)?(y):(x))
#define MAX(x, y) ((x)>(y)?(x):(y))
#define MINRADIUS (5)
/* I would like to make MINRADIUS configurable at runtime */

using namespace std;
using namespace Magick;

/* get the "hot" points on an image.  return them as a vector of Points.
 * right now we look for white pixels, which we hopefully got 
 * after doing an edge detect and thresholding on the image
 */
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

  /* we're looking for white */
  Color white("white");

  /* put all the white pixels on the vectore of points */
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


/* vote for the circle given by a, b, and r.
 * vote twice for the circle a, b, r, and once for each of its neighbors
 * because we're using ints in a real word 
 */
void vote(int a, int b, int r, int maxA, 
	  int maxB, int maxR,
	  vector<vector<vector< int > > > *votes){
  for(int i = MAX(0, a-1); i <= MIN(maxA, a+1); i++){
    for(int j = MAX(0, b-1); j <= MIN(maxB, b+1); j++){
      for(int k = MAX(0, r-1); k <= MIN(maxR, r+1); k++){
	if (votes->size() < i) votes->resize(i);
	if (votes[i]->size() < j) votes[i]->resize(j);
	if (votes[i][j]->size() < k) {
	  votes[i][j]->resize(k);
	  (*votes)[i][j][k] = 0;
	}
	(*votes)[i][j][k]++;
	if(i == 0 && j == 0 && k == 0) (*votes)[i][j][k]++;
      }
    }
  }
}

/* take the "hot" points on the image, and vote for their possible circles
 * return the 3d array of the votes (x and y position, and radius) 
 */
vector<vector<vector< int > > >  electCircles(vector<Point> *points,
					      int maxRadius, 
					      Point lowerRightCorner) {
  vector<vector<vector< int > > > votes;
  int maxA = lowerRightCorner.getX();
  int maxB = lowerRightCorner.getY();
  /* iterate through the points */
  for(vector<Point>::iterator p = points->begin(); p != points->end(); ++p){
    int i = p->getX();
    int j = p->getY();
    /* vote for expanding circles around that point */
    for(int r = MINRADIUS; r <= maxRadius; r++){
      int a = r;
      /* get pairs of values a, b that are on a circle of radius r */
      do{
	int b = sqrt((r^2) - ((a)^2));
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
	a--;
      } while (b < a); /* we've hit 45 degrees, all the points have been hit */
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

Circle clearArea(int x, int y, int r, vector<vector<vector< bool > > > *marked,
		 vector<vector<vector< int > > > *votes, double threshold){
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
  int maxCircleValue = circlevalue(returnCircle, (*votes)[x][y][r]);

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
    int maxB = votes[a]->size();
    int maxR = votes[a][b]->size();
    /* go through this circle's neighbors in the vote space */
    for(int i = MAX(0, a-1); i <= MIN(maxA, a+1); i++){
      for(int j = MAX(0, b-1); j <= MIN(maxB, b+1); j++){
	for(int k = MAX(0, r-1); k <= MIN(maxR, r+1); k++){
	  /* get their relavant data */
	  Point tempPoint = Point(i, j);
	  Circle newCircle = Circle(tempPoint, k);
	  double weight = circleValue(newCircle, votes[i][j][k]);
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
  return returnCircle;
}

/* get a list of the circles which have a heuristic value above threshold
 * clusters of contiguous circles in the votespace which are all above
 * the threshold will be considered one circle.  We will use the one with
 * the highest heuristic value.
 */
vector<Circle> chooseWinners(const vector<vector<vector< int > > > votes,
			     double threshold){
  vector<vector<vector< bool> > > marked;
  vector<Circle> circleList;

  /* clear the circles we've seen */
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

  /* iterate through the votespace */
  for(int i = 0; i < votes.size(); i++){
    for(int j = 0; j < votes[i].size(); j++){
      for(int r = 0; r < votes[i][j].size(); r++){
	Point circleCenter = Point(i, j);
	Circle circle = Circle(circleCenter, r);
	int value = circleValue(circle, votes[i][j][r]);
	/* if we find a circle with a heursitic above the threshold 
	   that hasn't already been counted in a cluster then find the cluster
	   and add the circle with the best heuristic in the cluster to
	   our list */
	if(value > threshold && marked[i][j][r] == false){
	  marked[i][j][r] = true;
	  circleList.push_back(clearArea(i, j, r, &marked, &votes, threshold));
	}
      }
    }
  }
  return circleList;
}
