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
  return ($limit < $value / $r);
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
$maxr = $shortside >> 1;

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

#for($i = 0; $i < $height; $i++){
#  push @rows, [()];
#}
#for($i = 0; $i < $width; $i++){
#  push @columns, [()];
#}

# i and j loops iterate through the image
for($i = 0; $i <= $width; $i++) {
  print "$i of $width evaluated $pointsevaluated points\n";
  for($j = 0; $j <= $height; $j++) {
    $pix = $im->getPixel($i,$j);
    if ($pix == 1){
	$pointsevaluated++;
	push(@{$rows[$j]}, $i);
	push(@{$columns[$i]}, $j);
    }
  }
}

for($i = 0; $i <= $width; $i++){
  print "working on row $i\n";
  @unseenX = @{$rows[$i]};
  foreach $Xa (@{$rows[$i]}){
    while($unseenX[0] - $Xa < 3 && @unseenX){
      shift @unseenX;
    }
    foreach $Xb (@unseenX){
      $x = ($Xa + $Xb) >> 1;
      @unseenY = @{$columns[$x]};
      foreach $Ya (@{$columns[$x]}){
         while($unseenY[0] - $Ya < 3 && @unseenY) {
           shift @unseenY;
         }
         foreach $Yb (@unseenY){
#           print "data: $i, $Ya, $Yb, $Xa, $Xb, $x\n";
           if($Yb != $i){
             $slopeA = ($i - $Ya)/($Xa - $x);
	     $slopeB = ($i - $Yb)/($Xb - $x);
             $circleFit = ($slopeA - 1/$slopeB)**2;
#             print "slopes and circleFit: $slopeA, $slopeB, $circleFit\n";
             if($circleFit < 0.001){
                $y = ($Yb + $Ya) >> 1;
                $r = ($Yb - $Ya) >> 1;
                $searchspace[$x][$y][$r]++;
                $count = $searchspace[$x][$y][$r];
                if($count > 0 && $r > 0){
	          $histogram[$count]++;
	          $histogram[$count - 1]--;
	        }
#               print "possible circle at (", $x, ", ", ($Yb + $Ya)/2, ") R = ", ($Yb - $Ya)/2 , "\n";
             }
	   }
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

