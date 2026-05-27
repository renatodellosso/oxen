void outer(bool x) {
    void inner(bool x) {
        outer(false);
    }
    
    print x;
    if (x)
        inner(true);
}

outer(true);