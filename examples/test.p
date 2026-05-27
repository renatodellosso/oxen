void outer(int x) {
    void inner(bool y) {
        outer(0);
    }

    print x - 1;
    
    if (x)
        inner(true);

    print x + 1;
}

outer(5);