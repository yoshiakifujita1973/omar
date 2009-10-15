#!/usr/bin/perl

use GD;
require "houghlib.pl";

if ($#ARGV < 2){
  print "Usage: perl hough.pl <file> <number of circles> <minimum radius>\n";
  exit;
}

$filename = $ARGV[0];
$circlestofind = $ARGV[1];
$minradius = $ARGV[2];

@points = getPoints($filename);
print "I have ", scalar @points, " points.\n";

@votes = electCircles(\@points);
print "Election complete!\n";

@circles = findWinners(\@votes, $circlestofind, $minradius);

print "Run the following command to overlay the found circles on your picture:\n\n";

print "convert $filename -fill none -stroke red -strokewidth 1 ";
foreach $circle (@circles) {
  ($max, $x, $y, $r) = @$circle;
  print "-draw 'circle $x, $y ", $x - $r, ", $y' ";
}
print "found.png\n";

