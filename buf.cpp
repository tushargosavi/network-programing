#include <iostream>
#include <string>

class buf {
public:
    buf(size_t len) {
        ptr = (unsigned char *)malloc(len);
        write_pos = 0;
        read_pos = 0;
    }

private:
    unsigned char *ptr;
    size_t write_pos;
    size_t read_pos;
};
