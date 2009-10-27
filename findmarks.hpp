#ifndef FINDMARKS_HPP
#define FINDMARKS_HPP

#include<stdio.h>
#include<vector>
#include<unistd.h>
#include<math.h>
//#include<stdlib>
#include<iostream>
#include<Magick++.h>
#include"geometricPrimitives.hpp"

using namespace std;

vector<Circle> findmarks(Magick::Image);

#endif
