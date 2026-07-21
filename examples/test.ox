int a;
a = 2;
int b = 5;

bool isPositive(int x) {
  if (x > 0) {
    return true;
  } else {
    return false;
  }
}

print "a is positive: " + isPositive(a);

while (isPositive(b)) {
  print "b is: " + b;
  b = b - 1;
}