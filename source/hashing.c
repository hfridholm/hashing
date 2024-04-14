#include "hashing.h"

char* algorithm = NULL;
char* filepath  = NULL;

/*
 * RETURN (int status)
 * - 0 | Success!
 * - 1 | No inputted algorithm
 * - 2 | Algorithm not supported
 */
int algorithm_hash(char* hash, const void* message, size_t size, const char* algorithm)
{
  if(algorithm == NULL) return 1;

  if(!strcmp(algorithm, "sha256"))
  {
    sha256(hash, message, size);
  }
  else if(!strcmp(algorithm, "md5"))
  {
    md5(hash, message, size);
  }
  else return 2;

  return 0; // Success!
}

/*
 * PARAMS
 * - const void* message   | The message to hash
 * - size_t size           | The amount of bytes (8 bits)
 * - const char* algorithm | The used hash algorithm
 */
void message_algorithm_hash_print(const void* message, size_t size, const char* algorithm)
{
  char hash[65];
  memset(hash, '\0', sizeof(char) * 65);

  int status = algorithm_hash(hash, message, size, algorithm);

  switch(status)
  {
    case 0: printf("%s\n", hash); break;

    case 1: printf("No inputted algorithm\n"); break;

    case 2: printf("Algorithm not supported\n"); break;
  }
}

/*
 * Output text when something is wrong with an option
 */
void opt_wrong(void)
{
  if(optopt == 'a')
    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
  else if(isprint(optopt))
    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
  else
    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
}

/*
 * PARAMS
 * - int opt | The option to parse
 *
 * RETURN (int status)
 * - 0 | Success!
 * - 1 | Something wrong with option
 */
int opt_parse(int opt)
{
  switch(opt)
  {
    case 'a': algorithm = optarg; break;

    case '?': opt_wrong(); return 1;

    default : abort();
  }
  return 0; // Success!
}

/*
 * PARAMS (same as main)
 * - int argc     | The amount of arguments
 * - char* argv[] | The array of arguments
 */
int args_parse(int argc, char* argv[])
{
  opterr = 0;

  int opt;
  while((opt = getopt(argc, argv, "a:")) != -1)
  {
    if(opt_parse(opt) != 0) return 1;
  }

  // Non-option argument was supplied
  if(optind < argc) filepath = argv[optind];

  // Assignes default algorithm if not specified
  if(algorithm == NULL) algorithm = "sha256";

  return 0; // Success!
}

/*
 * Get the size of the data in a file at the inputted path
 *
 * PARAMS
 * - const char* filepath | The path to the file
 *
 * RETURN (size_t size)
 * - 0  | Error
 * - >0 | Success!
 */
size_t file_size(const char* filepath)
{
  FILE* stream = fopen(filepath, "rb");

  if(stream == NULL) return 0;

  fseek(stream, 0, SEEK_END); 

  size_t size = ftell(stream);

  fseek(stream, 0, SEEK_SET); 

  fclose(stream);

  return size;
}

/*
 * Read data from file and store it at the inputted address
 *
 * PARAMS
 * - void* pointer        | The address to store the read data
 * - size_t size          | The size of the data to read
 * - size_t nmemb         | The size of the chunks
 * - const char* filepath | The path to the file
 *
 * RETURN (same as fread)
 * - 0  | Error
 * - >0 | Success!
 */
int file_read(void* pointer, size_t size, size_t nmemb, const char* filepath)
{
  FILE* stream = fopen(filepath, "rb");

  if(stream == NULL) return 0;

  int status = fread(pointer, size, nmemb, stream);

  fclose(stream);

  return status;
}

/*
 * RETURNS
 * - 0 | Success!
 * - 1 | Failed to parse arguments
 * - 2 | Failed to read inputted file
 * - 3 | Failed to input from stdin
 */
int main(int argc, char* argv[])
{
  if(args_parse(argc, argv) != 0) return 1;

  if(filepath != NULL)
  {
    size_t size = file_size(filepath);

    char message[size];

    if(file_read(message, size, 1, filepath) == 0)
    {
      printf("Failed to read inputted file\n");

      return 2;
    }

    message_algorithm_hash_print(message, size, algorithm);
  }
  else
  {
    char message[1024];
    memset(message, '\0', sizeof(message));

    if(fgets(message, sizeof(message), stdin) == NULL)
    {
      printf("No message was inputted\n");

      return 3;
    }

    message_algorithm_hash_print(message, strlen(message), algorithm);
  }

  return 0; // Success!
}
