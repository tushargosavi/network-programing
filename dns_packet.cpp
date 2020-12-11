
struct dns_header {
  unsigned short id;
  /* flag1 
   * QR <- query
   * Opcode <- 4 bytes
   * AA <- Authorative
   * TC <- Truncation
   * RD <- Recursion desired
  */
  union {
    unsigned char value;
    struct {
      unsigned char query: 1;
      unsigned char opcode : 4;
      unsigned char aa  : 1;
      unsigned char tc : 1;
      unsigned char rd : 1;
    } bits;
  } flags1;

  /*
   * RA <- Recursion available (1 bit)
   * Z <- Reserved (3 bits)
   * RCODE <- 4 bits)
   */
  union {
    unsigned char flag2;
    struct {
      unsigned char ra : 1;
      unsigned char z : 3;
      unsigned char rcode : 4;
    } bits;
  } flag2;

  unsigned short qdcount;
  unsigned short ancount;
  unsigned short nscount;
  unsigned short arcount;
};

struct dns_packet {
  struct dns_header header;
}

