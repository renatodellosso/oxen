void add(bool a) {
    print a;
    if (a) {
        add(false);
    }
}

add(true);