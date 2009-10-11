#!/usr/local/bin/perl

use GD;

$circlestofind = $ARGV[1];

sub max {
  return $_[0]>$_[1]?$_[0]:$_[1];
}

sub min {
  return $_[0]<$_[1]?$_[0]:$_[1];
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
        for(my $r = max(0, $Sr - 1); $r <= min($Sr + 1, $#{$searchspace[$i][$j]}); $r++){
          if($searchspace[$i][$j][$r] > $limit && $marked[$i][$j][$r] == 0){
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
      for(my $r = 3; $r <= $#{$searchspace[$i][$j]}; $r++){
	if ($searchspace[$i][$j][$r] > $limit && $marked[$i][$j][$r] != 1){
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

# i and j loops iterate through the image
for($i = 0; $i <= $width; $i++) {
  print "$i of $width evaluated $pointsevaluated points\n";
  $lastk = min($i + $maxr,($width + $i) >> 1);
  for($j = 0; $j <= $height; $j++) {
    $pix = $im->getPixel($i,$j);
    if ($pix == 1){
      $pointsevaluated++;
      $lastl = min($j + $maxr, ($height + $j) >> 1);
      # k and l loops iterate through centerpoints at a given "hot" pixel
      for($k = $i >> 1; $k < $lastk; $k++){
        $x = $i - $k;
	$x2 = $x * $x;
	for($l = $j >> 1; $l < $lastl; $l++){
          $y = $j - $l;
          $rsquared = $x2 + $y * $y;
	  if ($rsquared < $maxr2) {
	    # find the radius and vote at position k, l, r
            $r = int(sqrt($rsquared)+0.5);
	    if($r > 0){
              $searchspace[$k][$l][$r]++;
              $count = $searchspace[$k][$l][$r];
            }
            # keeping a histogram of how many points have how many votes might be useful.
   	    if($count > 0 && $r > 0){
	      $histogram[$count]++;
	      $histogram[$count - 1]--;
	    }
	  }
        }
      }
    }
  }
}

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
$limit = int($firstzero + $selectivity * $std); # used to do $avg + $selectivity * $std;
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
  $minallowedratio = 3;
  # We're going to kick out the worst-fit circles if we found too many.
  while($#goodcircles + 1 > $circlestofind) {
    $#stillgood = -1;
 #   print $#goodcircles, "+1 good circles\n";
    $lowestcircle = 0;
    $minratio = 5;
    while($circle = pop(@goodcircles)){
      ($votes, $x, $y, $r) = @$circle;
      # let's see how many votes we got in comparison to the radius to find "good" circles
      # 1 = ca 30%, 2 = ca 60%, pi = 100%
      $circleratio = $votes/$r;
      # if the match is better than our worst match so far keep it
      if($circleratio > $minratio){
        push(@stillgood, $circle);
      # if it is the worst so far, but is better than the worst we'll accept, store it
      } elsif($circleratio > $minallowedratio) {
	if($lowestcircle != 0){
          push(@stillgood, $lowestcircle);
	}
        $lowestcircle = $circle;
        $minratio = $circleratio;
      } # doing nothing drops the circle -- it was a bad match in general!
    }
    @goodcircles = @stillgood;
  }
  $limit-=10;
#  print "$circlestofind and ", $#goodcircles + 1, "\n";
} while($circlestofind > $#goodcircles + 1);

foreach $circle (@goodcircles) {
  ($max, $x, $y, $r) = @$circle;
  print "circle found at $x, $y with radius $r and $max votes\n";
}

foreach $circle (@goodcircles) {
  ($max, $x, $y, $r) = @$circle;
  print "-draw 'circle $x, $y ", $x - $r, ", $y' ";
}
print "\n";

