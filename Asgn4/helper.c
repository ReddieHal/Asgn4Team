
int checksum(char *blockHead) {
    int i, value = 0;

    for (i = 0; i < BLOCKSIZE; i++) {
        value += (unsigned char)blockHead[i];
    }

    return value;
}