#include <stdio.h>
#include <string.h>

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

  char hash[65];
  memset(hash, '\0', sizeof(char) * 65);

  sha256(hash, string, sizeof(char) * strlen(string));

  printf("%s\n", hash);

  return 0;
}
