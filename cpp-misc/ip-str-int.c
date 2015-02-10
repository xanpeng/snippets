#include <stdio.h>
#include <stdlib.h>

void print_ip(int ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF; 
    printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);        
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Usage: %s <INT IP>\n", argv[0]);
    exit(-1);
  }
  int ip_int = atoi(argv[1]);
  print_ip(ip_int);
}
