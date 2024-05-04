#include "hashing.h"

char* algorithm = NULL;

char** paths;
size_t pamount = 0;

/*
 * RETURN (int status)
 * - 0 | Success!
 * - 1 | No inputted algorithm
 * - 2 | Algorithm not supported
 */
int message_hash(char* hash, const void* message, size_t size)
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
 */
void message_hash_print(const void* message, size_t size)
{
  char hash[65];
  memset(hash, '\0', sizeof(char) * 65);

  int status = message_hash(hash, message, size);

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
  {
    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
  }
  else if(isprint(optopt))
  {
    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
  }
  else
  {
    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
  }
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

  pamount = (argc - optind);

  paths = malloc(sizeof(char*) * pamount);

  // Every non-option argument gets added as a file
  for(int index = 0; (optind + index) < argc; index++)
  {
    paths[index] = argv[optind + index];
  }

  // Assignes default algorithm if not specified
  if(algorithm == NULL) algorithm = "sha256";

  return 0; // Success!
}

/*
 * RETURNS
 * - 0 | Success!
 * - 1 | Failed to parse arguments
 */
int main(int argc, char* argv[])
{
  if(args_parse(argc, argv) != 0) return 1;

  // 1. Get all the FILES that will be hashed
  size_t amount = files_amount(paths, pamount, -1);

  char** files = files_create(amount, paths, pamount, -1);

  // 2. Read the files to the hash MESSAGE
  size_t size = files_size(files, amount);

  char message[size + 1];

  files_read(message, size, 1, files, amount);

  // 3. Hash the message and PRINT the checksum
  message_hash_print(message, size);

  // 4. FREE the used varaibles
  files_free(files, amount);

  free(paths);

  return 0; // Success!
}
