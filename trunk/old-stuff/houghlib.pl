use GD;

sub max {
  return $_[0]>$_[1]?$_[0]:$_[1];
}

sub min {
  return $_[0]<$_[1]?$_[0]:$_[1];
}

sub circlevalue{
  my $value = $_[0];
  my $i = $_[1];
  my $j = $_[2];
  my $r = $_[3];
  $r = 1 if $r == 0;
  return $value / $r;
}

sub inspec{
  my $value = $_[0];
  my $i = $_[1];
  my $j = $_[2];
  my $r = $_[3];
  my $limit = $_[4];
  return ($limit < circlevalue($value, $i, $j, $r));
}

# Votes for a circle.  
# Votes twice for the circle it was passed, and once for all its neighbors.
# This should smooth out the field a bit and help like circles glob together.
sub voteforcircle{
  my $x = $_[0];
  my $y = $_[1];
  my $r = $_[2];
  my $searchspace = $_[3];
  if($x == 50 && $y == 50 && $r == 45){
    print "voting for a good circle!\n";
  }
  if ($x<0 || $y < 0) {
    return; 
  }
# print "adding circle ($x, $y) $r\n";
  $searchspace->[$x][$y][$r]++;
  $count = $searchspace->[$x][$y][$r];
  # keeping a histogram of how many points have how many votes might be useful.
  if($count > 0){
     $histogram[$count]++;
     $histogram[$count - 1]--;
  }

  foreach $i (-1..1){
    foreach $j (-1..1){
      foreach $k (-1..1){
        $searchspace->[$x+$i][$y+$j][$r+$k]++;
        $count = $searchspace->[$x+$i][$y+$j][$r+$k];
        # keeping a histogram of how many points have how many votes might be useful.
        if($count > 0){
          $histogram[$count]++;
          $histogram[$count - 1]--;
        }
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
  my $searchspace = $_[0];
  my $marked = $_[1];
  my $Si = $_[2];
  my $Sj = $_[3];
  my $Sr = $_[4];
  my $limit = $_[5];
  my $minradius = $_[6];
  my $max;
  $max = $searchspace->[$Si][$Sj][$Sr];
  my @result = ($max, $Si, $Sj, $Sr);
  unshift(@searchqueue, [$Si, $Sj, $Sr]);
  while($#searchqueue > -1) {
    $sq_ref = pop @searchqueue;
    ($Si, $Sj, $Sr) = @$sq_ref;
    $marked->[$Si][$Sj][$Sr] = 1;
    for(my $i = max(0, $Si - 1); $i < min($Si + 1, $#{$searchspace}); $i++){
      for(my $j = max(0, $Sj - 1); $j < min($Sj + 1, $#{$searchspace->[$i]}); $j++){
        for(my $r = max($minradius, $Sr - 1); $r < min($Sr + 1, $#{$searchspace->[$i][$j]}); $r++){
          if(defined $searchspace->[$i][$j][$r] && 
             inspec($searchspace->[$i][$j][$r], $i, $j, $r, $limit) && $marked->[$i][$j][$r] == 0){
	    $marked->[$i][$j][$r] = 1;
	    unshift(@searchqueue,[$i, $j, $r]);
	    if($searchspace->[$i][$j][$r] > $max){
	      $max = $searchspace->[$i][$j][$r];
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
  my $searchspace = $_[0];
  my $limit = $_[1];
  my $minradius = $_[2];
  my $max = 0;
  my @marked;
  my @circles;
  my $x, $y, $z;
  for($x = 0; defined $searchspace->[$x]; $x++){
    for($y = 0; defined $searchspace->[$x][$y]; $y++){
      for ($z = 0; defined $searchspace->[$x][$y][$z]; $z++){
        $marked[$x][$y][$z] = 0;
      }
    }
  }
  for(my $i = 0; defined $searchspace->[$i]; $i++){
    for(my $j = 0; defined $searchspace->[$i][$j]; $j++) {
      for(my $r = $minradius; defined $searchspace->[$i][$j][$r]; $r++){
	if (inspec($searchspace->[$i][$j][$r], $i, $j, $r, $limit) && $marked[$i][$j][$r] != 1){
	  push(@circles, [cleararea($searchspace, \@marked, $i, $j, $r, $limit, $minradius)]);
#	  ($max, $mi, $mj, $mr) = @$foundcircle;
        }
      }
    }
  }
  return @circles;
}


sub getPoints{
  my $file = $_[0];
  my @points;
  # get image from file
  $im = GD::Image->new($file) || die "Cannot open file $file.";

  # get the image's dimensions in pixels
  ($width, $height) = $im->getBounds();

  # i and j loops iterate through the image
  # and we put each pixel with a value of 1 into a stack of points.
  for(my $i = 0; $i <= $width; $i++) {
    for(my $j = 0; $j <= $height; $j++) {
      my $pix = $im->getPixel($i,$j);
      if ($pix == 1){
	push(@points, [($i, $j)]);
      }
    }
  }
  return @points;
}

sub electAbe{
  my @points = @$_[0];
  my @searchspace;

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
	    voteforcircle($Xd, $Yd, $r, \@searchspace);
            voteforcircle($Xe, $Ye, $r, \@searchspace);
          }
        }
      } 
    }
  }
  return @searchspace;
}

sub electStu{
  my @points =  @$_[0];
  my @rows;
  my @colums; 
  my @searchspace;

  foreach $point (@points) {
    my ($x, $y) = @$point;
    push(@{$rows[$j]}, $i);
    push(@{$columns[$i]}, $j);
  }

  for(my $i = 0; $i <= $width; $i++){
    print "working on row $i\n";
    my @unseenX = @{$rows[$i]};
    foreach $Xa (@{$rows[$i]}){
      while($unseenX[0] - $Xa < 3 && @unseenX){
        shift @unseenX;
      }
      foreach $Xb (@unseenX){
        $x = ($Xa + $Xb) / 2;
        @unseenY = @{$columns[$x]};
        foreach $Ya (@{$columns[$x]}){
          while($unseenY[0] - $Ya < 3 && @unseenY) {
            shift @unseenY;
          }
          foreach $Yb (@unseenY){
            if($Yb != $i){
              $slopeA = ($i - $Ya)/($Xa - $x);
              $slopeB = ($i - $Yb)/($Xb - $x);
              $circleFit = ($slopeA - 1/$slopeB)**2;

              if($circleFit < 0.001){
                 $y = ($Yb + $Ya) / 2;
                 $r = ($Yb - $Ya) / 2;
	         voteforcircle($x, $y, $r, \@searchspace);
              }
	    }
          }
        }
      }
    }
  } 
  return @searchspace;
}

sub electHough{
  my $points = $_[0];

  my @searchspace;
    push @rMatrix, 0;
    push @yMaxtrix, [ @rMatrix ];
    push @searchspace, [ @yMatrix ];
  my $height = 0;
  my $width = 0;
  my $maxr;
  foreach $point (@$points){
    ($x, $y) = @$point;
    $width = $x if $x > $width;
    $height = $y if $y > $height;
  }
  $maxr = (($height<$width)?$height:$width)/2;
  $maxr2 = $maxr**2;

  foreach $point (@$points){
    my ($i, $j) = @$point;
    my $lastk = min($i + $maxr,($width + $i) / 2);
    my $lastl = min($j + $maxr, ($height + $j) / 2);
    # k and l loops iterate through centerpoints at a given "hot" pixel
    for(my $k = max($i / 2, $i - $maxr); $k < $lastk; $k++){
      my $x = $i - $k;
      my $x2 = $x ** 2;
      for(my $l = max($j / 2, $j - $maxr); $l < $lastl; $l++){
        my $y = $j - $l;
        if($count > 0){
          $histogram[$count]++;
          $histogram[$count - 1]--;
        }

        my $rsquared = $x2 + $y * $y;
        if ($rsquared < $maxr2) {
        # find the radius and vote at position k, l, r
          my $r = int(sqrt($rsquared)+0.5);
          if($r > 0){
             voteforcircle($x, $y, $r, \@searchspace);
          }
        }
      }
    }
  }
  print "", scalar @searchspace, "\n";
  return @searchspace;
}

sub electCircles{
  return electHough($_[0], $_[1], $_[2], $_[3]);
}

sub findWinners{
  my $votespace = $_[0];
  my $circlestofind = $_[1]; 
  my $minradius = $_[2];
  my $limit = 2000;
  my @goodcircles = [];

  do {  
    @goodcircles = findseeds($votespace, $limit, $minradius);
    $limit *= 0.90;
#    print "Working with ", scalar @goodcircles, " need $circlestofind\n";
  } while ((scalar @goodcircles) < $circlestofind);
#  print "clearing them out";

  foreach $circle (@goodcircles){
    my ($votes, $x, $y, $r) = @$circle;
    my $max, $min;
    my $value = circlevalue($votes, $x, $y, $r);
    if(defined $max){
      $max = $value if ($max < $value);
      $min = $value if ($min > $value);
    } else {
      $max = $value;
      $min = $value;
    } 
  }

  my $maxratio = $max;
  my $minratio = $min;
  my @stillgood;

  while(scalar @goodcircles > $circlestofind) {    
    $#stillgood = -1;
    my $lowestcircle = 0;
    while($circle = pop(@goodcircles)){
      my ($votes, $x, $y, $r) = @$circle;
      $circlevalue = circlevalue($votes, $x, $y, $r);
      $maxratio = ($circleratio > $maxratio)?$circleratio:$maxratio;
      # if the match is better than our worst match so far keep it
      if($circleratio > $minratio){ 
        push(@stillgood, $circle);
        # if it is the worst so far, but is better than the worst we'll 
        # set it aside so we can drop it at the end.
      } else {
        if($lowestcircle != 0){
        push(@stillgood, $lowestcircle);
        }
        $lowestcircle = $circle;
        $minratio = $circleratio;
      }
    }
  @goodcircles = @stillgood;
  }
  return @goodcircles;
}

return true;
