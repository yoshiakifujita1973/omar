#ifndef HOUGH_HPP
#define HOUGH_HPP
#include<vector>
#include<Magick++.h>
#include"geometricPrimitives.hpp"

std::vector<Circle> findCircles(Magick::Image, int, int, int);

#endif
