int outer(int captured) {
  int inner(int n) {
    if (n == 1) return captured;
    else return inner(n - 1);
  }
  return inner(3);
}
print outer(7);