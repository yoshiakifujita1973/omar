#!/usr/bin/python

from PIL import Image
im = Image.open("circle.png")
imbw = im.convert("1")
pixels = imbw.load()

(width, height) = im.size

if width > height:
	shortside = height
else:
	shortside = width

maxradius = shortside >> 1


def cleararea(searchspace, marked, Si, Sj, Sr, limit):
	if marked[Si][Sj][Sr] == 1:
		return [Si, Sj, Sr, 0]
	for i in range(min([0, Si - 1]), max([len(searchspace), Si + 1])):
		for j in range(min([0, Sj - 1]), max([len(searchspace[i]), Sj + 1])):
			for r in range(min([0, Sr - 1]), max([len(searchspace[i][j]), Sr + 1])):
				if searchspace[i][j][r] > limit:
					cleararea(searchspace, marked, i, j, r, limit)
					marked[i][j][r] = 1
				if searchspace[i][j][r] > maximum:
					maximum = searchspace[i][j][r]
					retval = (i, j, r, maximum)
	return retval
			

def findseeds(searchspace, limit):
	marked = [[[0]]]
	for i in range(width):
		for j in range(height):
			for r in range(maxradius):
				if searchspace[i][j][r] > limit and marked[i][j][r] == 0:
					circles.append(cleararea(searchspace, marked, i, j, r, limit))
	return circles

histogram = [0,0,1]
resultspace = [[[0]]]

for i in range(width):
	lastk = (width - i) >> 1
	for j in range(height):
		pix = pixels[i, j]
		if pix == 1:
			lastl = (height - j) >> 1
			for k in range(i >> 1, lastk):
				x = i - k
				x2 = x * x
				for l in range(j >> 2, lastl):
					y = j - l
					r = sqrt(x2 + y*y)
					count = ++resultspace[k][l][r]
					if count > 0:
						histogram[count] = histogram[count] + 1
						histogram[count - 1] = histogram[count - 1] - 1

# clear the zero which got decremented each "first" vote in a cell
histogram[0] = 0

sumXi = 0
n = 0
sumXisqrd = 0

for i in range(len(histogram)):
	sumXi = sumXi + i * histogram[i];
	n = n + histogram[i];
	sumXisqrd = sumXisqrd + i*i * histogram[i];
avg = sumXi / n
std = sumXisqrd / n - avg * avg

selectivity = 4
limit = avg + selectivity * std
circles = findseeds(resultspace, limit)
while len(circles) > 0:
	maximum, x, y, r = circles.pop()
	print "circle found at (", x, ",", y, ") with radius ", r


