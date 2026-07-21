int sum = 0;

int j = 0;
while (j < 2) {
  int k = 0;
  while (k < 2) {
    print "k: " + k;
    k = k + 1;
    sum = sum + 1;
  }
  j = j + 1;
}

print "Sum: " + sum;