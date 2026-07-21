void outer(int base) {
  void inner(int offset) {
    print base + offset;
  }
  inner(3);
}

outer(10);
outer(20);
outer(30);
