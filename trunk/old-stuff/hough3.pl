#!/usr/local/bin/perl

use GD;

$circlestofind = $ARGV[1];
if ($#ARGV < 2){
  print "Usage: perl hough.pl <file> <number of circles> <minimum radius>\n";
  exit;
}
$minradius = $ARGV[2];

sub max {
  return $_[0]>$_[1]?$_[0]:$_[1];
}

sub min {
  return $_[0]<$_[1]?$_[0]:$_[1];
}

sub inspec{
  my $value = $_[0];
  my $i = $_[1];
  my $j = $_[2];
  my $r = $_[3];
  my $limit = $_[4];
  return ($limit < $value / $r**2);
}


sub addtosearchspace{
  my $x = $_[0];
  my $y = $_[1];
  my $r = $_[2];
  if ($x<0 || $y < 0) {
    return; 
  }
# print "adding circle ($x, $y) $r\n";
  foreach $i (-1..1){
    foreach $j (-1..1){
      $searchspace[$x+$i][$y+$j][$r]++;
      $count = $searchspace[$x+$i][$y+$j][$r];
      # keeping a histogram of how many points have how many votes might be useful.
      if($count > 0){
        $histogram[$count]++;
        $histogram[$count - 1]--;
      }
    }
  }
}

# Thanks Wolfram Alpha!  
# This returns the  x values that a circle bisected a line j = m*i+b and 
# whose edge contains the point (x, y) has when the circle has a radius of maxr.
sub getXrange{
  my $x = $_[0];
  my $y = $_[1];
  my $m = $_[2];
  my $b = $_[3];
  my $r = $_[4];
  my $minx = (-sqrt(-$b**2 - 2*$b*$m*$x + 2*$b*$y + $m**2*$r**2 - $m**2*$x**2 + 2*$m*$x*$y + $r**2
                 - $y**2)
           - $b*$m+$m*$y + $x)/($m**2 + 1);
  my $maxx = (sqrt(-$b**2 - 2*$b*$m*$x + 2*$b*$y + $m**2*$r**2 - $m**2*$x**2 + 2*$m*$x*$y + $r**2
                 - $y**2)
           - $b*$m+$m*$y + $x)/($m**2 + 1);
  return ($minx, $maxx);
}


# This function takes a location in the voting space, looks at its neighbors, and if they are above the
# threshold and not yet marked, adds them to the evaluation queue
sub cleararea {
#  my @searchspace = @$_[0];
#  my $marked = $_[1];
  my $Si = $_[2];
  my $Sj = $_[3];
  my $Sr = $_[4];
  my $limit = $_[5];
  my $max;
#  my $dmax;
  
  $max = $searchspace[$Si][$Sj][$Sr];
  my @result = ($max, $Si, $Sj, $Sr);
  unshift(@searchqueue, [$Si, $Sj, $Sr]);
  while($#searchqueue > -1) {
    $sq_ref = pop @searchqueue;
    ($Si, $Sj, $Sr) = @$sq_ref;
    $marked[$Si][$Sj][$Sr] = 1;
    for(my $i = max(0, $Si - 1); $i <= min($Si + 1, $#searchspace); $i++){
      for(my $j = max(0, $Sj - 1); $j <= min($Sj + 1, $#{$searchspace[$i]}); $j++){
        for(my $r = max($minradius, $Sr - 1); $r <= min($Sr + 1, $#{$searchspace[$i][$j]}); $r++){
          if(inspec($searchspace[$i][$j][$r], $i, $j, $r, $limit) && $marked[$i][$j][$r] == 0){
	    $marked[$i][$j][$r] = 1;
	    unshift(@searchqueue,[$i, $j, $r]);
	    if($searchspace[$i][$j][$r] > $max){
	      $max = $searchspace[$i][$j][$r];
	      @result = ($max, $i, $j, $r);
	    }
          }
        }
      }
    }
  }
  return @result;
}


# This function walks through the vote space and "seeds" the cleararea function with locations that are above
# a limit but haven't been marked as seen yet.

sub findseeds {
  #my @searchspace = $_[0];
  my $limit = $_[1];
  my $max;
  for(my $i = 0; $i <= $#searchspace; $i++){
#    print "i = $i and searching j to ", $#{$searchspace[$i]}, "\n";
    for(my $j = 0; $j <= $#{$searchspace[$i]}; $j++) {
#      print "  j = $j and searching r to ", $#{$searchspace[$i][$j]}, "\n";
      for(my $r = $minradius; $r <= $#{$searchspace[$i][$j]}; $r++){
	if (inspec($searchspace[$i][$j][$r], $i, $j, $r, $limit) && $marked[$i][$j][$r] != 1){
	  push(@circles, [cleararea(\@searchspace, \@marked, $i, $j, $r, $limit)]);
#	  ($max, $mi, $mj, $mr) = @$foundcircle;
        }
      }
    }
  }
  return @circles;
}

# get image from file
$im = GD::Image->new($ARGV[0]) || die "no dice on $ARGV[0]";

# get the images dimensions in pixels
($width, $height) = $im->getBounds();

print "size: $width, $height\n";
# Find the shorter dimension
$shortside = $width>$height?$height:$width;

# assume the max radius is half of the shorter dimension
# and square it for comparison purposes (to avoid having to do a square root all the time)
$maxr = $shortside / 2;

# Create a 3D array which looks like this:
# @searchspace[$width][$height][$shortside/2];
for($r = 0; $r < $maxr; $r++){
  push @rMatrix, 0;
}
for($y = 0; $y < $height; $y++){
  push @yMaxtrix, [ @rMatrix ];
}
for($x = 0; $x < $width; $x++){
  push @searchspace, [ @yMatrix ];
}

$maxr2 = $maxr * $maxr;

# i and j loops iterate through the image
for($i = 0; $i <= $width; $i++) {
#  print "$i of $width evaluated $pointsevaluated points\n";
  for($j = 0; $j <= $height; $j++) {
    $pix = $im->getPixel($i,$j);
    if ($pix == 1){
	$pointsevaluated++;
	push(@points, [($i, $j)]);
   }
  }
}

@pointsB = @points;
foreach $pointA (@points){
  pop @pointsB;
  print "points left ", scalar @pointsB, "\n";
  ($Xa, $Ya) = @$pointA;
  foreach $pointB (@pointsB){
    ($Xb, $Yb) = @$pointB;
    if($Xa != $Xb && $Ya != $Yb && ($Xa - $Xb)**2 + ($Ya - $Yb)**2 < $maxr2){
      $Xm = ($Xa + $Xb) / 2.0; 
      $Ym = ($Ya + $Yb) / 2.0;
      # slope of the line containing the center of a circle going through the two points.
      # which is perpendicular to the slop of the line defined by the two points.
      $m = -($Xa - $Xb) * 1.0 / ($Ya - $Yb);
      # the "b" value in y = mx+b;
      $b = $Ym - $m * $Xm;
      $r = sqrt(($Xa - $Xb)**2 + ($Ya - $Yb)**2);
      for($Xc = 1; $r<$maxr; $Xc++){
	$Xd = int($Xm + $Xc);
	$Xe = int($Xm - $Xc);
        $Yd = $m * $Xd + $b;
	$Ye = $m * $Xe + $b;
        $r = sqrt(($Xd - $Xa)**2 + ($Yd - $Ya)**2);
        if($Yd > 0 && $Yd < $height) {
	  addtosearchspace($Xd, $Yd, $r);
          addtosearchspace($Xe, $Ye, $r);
        }
      }
    } 
  }
}

#print "Not looking for anything, exiting here\n";
#exit;

print "$pointsevaluated points evaluated\n";

$histogram[0]=0;
for($i = 1; $i <= $#histogram; $i++){
  $sumXi += $i * $histogram[$i];
#  print "$i ", $histogram[$i], "\n";
  $n += $histogram[$i];
  $sumXisqrd += $i*$i*$histogram[$i];
  # let's record the first place in the histogram with zero votes, and start looking above that.
  if($histogram[$i] == 0 && $firstzero == 0){
    $firstzero = $i;
  }
}
$avg = $sumXi / $n;
$std = sqrt($sumXisqrd / $n - $avg * $avg);

$votesfromtop = 0;

for($i = $#histogram; $votesfromtop < $circlestofind*4; $i--) {
  $votesfromtop+= $histogram[$i];
}

print "vote average: $avg; vote STD: $std\n";

# choose a selectivity (in STDs away from the average)
$selectivity = 3;
$limit = 0.90 * 2 * 3.14159; # find circles that are 90% full.
#int($firstzero + $selectivity * $std); # used to do $avg + $selectivity * $std;
$distance = ($limit - $avg) / $std;

do { 
  $#marked = -1;
  for($x = 0; $x < $width; $x++){
    push @marked, [ @yMatrix ];
  }
  @oldcircles = @goodcircles;
  $#goodcircles = -1;  
  $#circles = -1;
  print "Have ", $#oldcircles + 1, " circles.  Need $circlestofind.  Looking above $limit votes\n";
  @circles = findseeds(\@searchspace, $limit);
  @goodcircles = @circles;
  print "found ", $#circles + 1, ".  Testing for validity.\n";
  # We're going to kick out the worst-fit circles if we found too many.
  $maxratio = 1;
  while($#goodcircles + 1 > $circlestofind) {    
    $#stillgood = -1;
 #   print $#goodcircles, "+1 good circles\n";
    $lowestcircle = 0;
    $minratio = $maxratio;
    while($circle = pop(@goodcircles)){
      ($votes, $x, $y, $r) = @$circle;
      # let's see how many votes we got in comparison to the radius to find "good" circles
      # 1 = ca 30%, 2 = ca 60%, pi = 100%
      $circleratio = $votes/$r;
#      print "$circleratio > $minratio is good? "; 
      $maxratio = ($circleratio > $maxratio)?$circleratio:$maxratio;
      # if the match is better than our worst match so far keep it
      if($circleratio > $minratio){
        push(@stillgood, $circle);
      # if it is the worst so far, but is better than the worst we'll set it aside so we can drop it at the end.
      } else {
	if($lowestcircle != 0){
          push(@stillgood, $lowestcircle);
	}
        $lowestcircle = $circle;
        $minratio = $circleratio;
      }
    }
    @goodcircles = @stillgood;
    print "still have ", $#goodcircles + 1, " circles\n";
  }
  $minratio = 2 * 3.14159;
  $limit = $limit * 0.90;
#  print "$circlestofind and ", $#goodcircles + 1, "\n";
} while($circlestofind > $#goodcircles + 1);

foreach $circle (@goodcircles) {
  ($max, $x, $y, $r) = @$circle;
  print "circle found at $x, $y with radius $r and $max votes\n";
}

print "convert $ARGV[0] -fill none -stroke red -strokewidth 1 ";
foreach $circle (@goodcircles) {
  ($max, $x, $y, $r) = @$circle;
  print "-draw 'circle $x, $y ", $x - $r, ", $y' ";
}
print "found.png\n";

