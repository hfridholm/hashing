/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-07-01
 */

#include "hashing.h"

static char doc[] = "hashing - compute hash algorithm checksum";

static char args_doc[] = "[FILE...]";

static struct argp_option options[] =
{
  { "algorithm", 'a', "STRING", 0, "The hash algorithm to use" },
  { "depth",     'd', "COUNT",  0, "The search depth in directory" },
  { "concat",    'c', 0,        0, "Concatonate the file hashes" },
  { 0 }
};

struct args
{
  char** args;
  size_t arg_count;
  char*  alg;
  int    depth;
  bool   concat;
};

struct args args =
{
  .args      = NULL,
  .arg_count = 0,
  .alg       = "sha256",
  .depth     = -1,
  .concat    = false
};

/*
 * This is the option parsing function used by argp
 */
static error_t opt_parse(int key, char* arg, struct argp_state* state)
{
  struct args* args = state->input;

  switch(key)
  {
    case 'c':
      args->concat = true;
      break;

    case 'd':
      args->depth = arg ? atoi(arg) : -1;
      break;

    case 'a':
      args->alg = arg;
      break;

    case ARGP_KEY_ARG:
      args->args = realloc(args->args, sizeof(char*) * (state->arg_num + 1));

      if(!args->args) return ENOMEM;

      args->args[state->arg_num] = arg;

      args->arg_count = state->arg_num + 1;
      break;

    case ARGP_KEY_END:
      // Use this line of code if the user has to input a file
      // if(state->arg_num < 1) argp_usage(state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

/*
 * Create hash from message block using hash algorithm
 *
 * RETURN (int status)
 * - 0 | Success!
 * - 1 | No inputted algorithm
 * - 2 | Algorithm not supported
 */
static int message_hash(char* hash, const void* message, size_t size)
{
  if(!args.alg) return 1;

  if(!strcmp(args.alg, "sha256"))
  {
    sha256(hash, message, size);
  }
  else if(!strcmp(args.alg, "md5"))
  {
    md5(hash, message, size);
  }
  else return 2;

  return 0; // Success!
}

/*
 * Create a concatonated hash from files and print it
 *
 * PARAMS
 * - char** files | The files to create hash from
 * - size_t count | The number of files
 */
static void files_concat_hash_print(char** files, size_t count)
{
  size_t size = files_size(files, count);

  char message[size + 1];

  files_read(message, size, 1, files, count);

  char hash[64 + 1];
  memset(hash, '\0', sizeof(hash));

  message_hash(hash, message, size);

  printf("%s\n", hash);
}

/*
 * PARAMS
 * - char*       hash     |
 * - size_t      size     |
 * - const char* filepath |
 *
 * RETURN (same as message_hash)
 */
static int file_hash(char* hash, size_t size, const char* filepath)
{
  char message[size + 1];
  memset(message, '\0', sizeof(message));

  file_read(message, size, 1, filepath);

  return message_hash(hash, message, size);
}

/*
 * PARAMS
 * - char*  message |
 * - size_t size    |
 *
 * RETURN (size_t length)
 */
static size_t message_input(char* message, size_t size)
{
  size_t length;

  for(length = 0; length < size; length++)
  {
    char symbol = fgetc(stdin);

    message[length] = symbol;

    if(symbol == EOF || symbol == '\0') break;
  }

  message[length] = '\0';

  return length;
}

/*
 * PARAMS
 * - char* hash |
 *
 * RETURN (same as message_hash)
 */
static int stdin_hash(char* hash)
{
  char message[1024];
  memset(message, '\0', sizeof(message));

  size_t size = message_input(message, sizeof(message) - 1);

  return message_hash(hash, message, size);
}

/*
 * Create hashes from files and print them
 *
 * PARAMS
 * - char** files | The files to create hash from
 * - size_t count | The number of files
 */
static void files_separate_hash_print(char** files, size_t count)
{
  char hash[64 + 1];
  memset(hash, '\0', sizeof(hash));

  for(size_t index = 0; index < count; index++)
  {
    if(strcmp(files[index], "-") != 0)
    {
      size_t size = file_size(files[index]);

      file_hash(hash, size, files[index]);
    }
    else stdin_hash(hash);

    printf("%s  %s\n", hash, files[index]);
  }
}

static struct argp argp = { options, opt_parse, args_doc, doc };

/*
 * This is the main function
 */
int main(int argc, char* argv[])
{
  argp_parse(&argp, argc, argv, 0, 0, &args);

  size_t count = files_count(args.args, args.arg_count, args.depth);

  char** files = files_create(count, args.args, args.arg_count, args.depth);

  if(count == 0 && !files)
  {
    files = malloc(sizeof(char*));

    files[count++] = strdup("-");
  }

  if(args.concat)
  {
    files_concat_hash_print(files, count);
  }
  else files_separate_hash_print(files, count);

  files_free(files, count);

  if(args.args) free(args.args);

  return 0; // Success!
}
