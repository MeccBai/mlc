

export i32 fab(i32 i) {
    if (i == 1 || i == 2) {
        return 1;
    }
    else {
        return fab(i-1) + fab(i-2);
    }
    return 0;
}

