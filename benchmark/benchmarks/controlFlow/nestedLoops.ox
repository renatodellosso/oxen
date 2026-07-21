int total = 0;
int i = 0;

while (i < 20) {
  int j = 0;
  while (j < 15) {
    int k = 0;
    while (k < 10) {
      total = total + i + j + k;
      k = k + 1;
    }
    j = j + 1;
  }
  i = i + 1;
}

print total;
