#include "hashing.h"

int main(int argc, char* argv[])
{
  if(argc < 2)
  {
    printf("No string was inputted\n");
    return 1;
  }

  char* string = argv[1];

  printf("%s\n", string);

  char hash[64];

  printf("%s\n", sha256(hash, string, sizeof(char) * strlen(string)));

  return 0;
}
