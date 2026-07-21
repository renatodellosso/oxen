int weightedSum(int a, int b, int weight) {
  int sum = a + b;
  return sum * weight;
}

int i = 0;
int total = 0;
while (i < 200) {
  total = total + weightedSum(i, 2, 3);
  i = i + 1;
}

print total;
