/*
 * Compute hash algorithm checksum
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-02-14
 */

#define FILE_IMPLEMENT
#include "file.h"

#define SHA256_IMPLEMENT
#include "sha256.h"

#define MD5_IMPLEMENT
#include "md5.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <argp.h>

static char doc[] = "hashing - compute hash algorithm checksum";

static char args_doc[] = "[FILE...]";

static struct argp_option options[] =
{
  { "algorithm", 'a', "ALGORITHM", 0, "Hash algorithm to use" },
  { "depth",     'd', "DEPTH",     0, "Directory depth limit" },
  { "concat",    'c', 0,           0, "Concatonate files"     },
  { 0 }
};

struct args
{
  char** paths;
  size_t path_count;
  char*  algorithm;
  int    depth;
  bool   concat;
};

struct args args =
{
  .paths      = NULL,
  .path_count = 0,
  .algorithm  = "sha256",
  .depth      = 1,
  .concat     = false
};

/*
 * This is the option parsing function used by argp
 */
static error_t opt_parse(int key, char* arg, struct argp_state* state)
{
  struct args* args = state->input;

  switch (key)
  {
    case 'c':
      args->concat = true;
      break;

    case 'd':
      int depth = atoi(arg);

      if (depth == 0 || depth < -1) argp_usage(state);

      else args->depth = depth;
      break;

    case 'a':
      args->algorithm = arg;
      break;

    case ARGP_KEY_ARG:
      args->paths = realloc(args->paths, sizeof(char*) * (state->arg_num + 1));

      if (!args->paths) return ENOMEM;

      args->paths[state->arg_num] = arg;

      args->path_count = state->arg_num + 1;
      break;

    case ARGP_KEY_END:
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
 * - 0 | Algorithm not supported
 * - 1 | Created SHA256 hash
 * - 2 | Created MD5    hash
 */
static int message_hash_create(char* hash, const void* message, size_t size)
{
  if (!args.algorithm) return 0;

  if (strcmp(args.algorithm, "sha256") == 0)
  {
    sha256(hash, message, size);

    return 1;
  }
  else if (strcmp(args.algorithm, "md5") == 0)
  {
    md5(hash, message, size);

    return 2;
  }
  else return 0;
}

/*
 * Create a concatonated hash from files
 *
 * PARAMS
 * - char*  hash  | Pointer to hash memory
 * - char** files | Paths to files
 * - size_t count | Number of files
 *
 * RETURN (same as message_hash_create)
 */
static int files_hash_create(char* hash, char** files, size_t count)
{
  size_t files_size = files_size_get(files, count);

  char* message = malloc(sizeof(char) * files_size);

  if (!message)
  {
    printf("Files are too large\n");

    return 0;
  }

  size_t read_size = files_read(message, files_size, files, count);

  int status = message_hash_create(hash, message, read_size);

  free(message);

  return status;
}

/*
 * Create hash from file
 *
 * PARAMS
 * - char*       hash     | Pointer to hash memory
 * - const char* filepath | Path to file
 *
 * RETURN (same as message_hash_create)
 */
static int file_hash_create(char* hash, const char* filepath)
{
  size_t file_size = file_size_get(filepath);

  char* message = malloc(sizeof(char) * file_size);

  if (!message)
  {
    printf("Files are too large\n");

    return 0;
  }

  size_t read_size = file_read(message, file_size, filepath);

  int status = message_hash_create(hash, message, read_size);

  free(message);

  return status;
}

/*
 * Create a concatonated hash from files, and print it
 */
static int files_hash_print(char** files, size_t count)
{
  char hash[64 + 1];
  memset(hash, '\0', sizeof(hash));

  if (files_hash_create(hash, files, count) == 0)
  {
    printf("hashing: Failed to create files hash");
    return 1;
  }

  printf("%s\n", hash);

  return 0;
}

/*
 * Create hash from file, and print it
 */
static int file_hash_print(const char* file)
{
  char hash[64 + 1];
  memset(hash, '\0', sizeof(hash));

  if (file_hash_create(hash, file) == 0)
  {
    printf("hashing: Failed to create file hash\n");
    return 1;
  }

  printf("%s  %s\n", hash, file);

  return 0;
}

/*
 * Create seperate hashes from files, and print them
 */
static void file_hashes_print(char** files, size_t count)
{
  for (size_t index = 0; index < count; index++)
  {
    char* file = files[index];

    file_hash_print(file);
  }
}

/*
 * Input message from stdin
 *
 * PARAMS
 * - char*  message | Pointer to message memory
 * - size_t size    | Max size of message
 *
 * RETURN (size_t length)
 */
static size_t message_input(char* message, size_t size)
{
  size_t length;

  for (length = 0; length < size; length++)
  {
    char symbol = fgetc(stdin);

    message[length] = symbol;

    if (symbol == EOF || symbol == '\0') break;
  }

  return length;
}

/*
 * PARAMS
 * - char* hash | Pointer to hash memory
 *
 * RETURN (same as message_hash_create)
 */
static int stdin_hash_create(char* hash)
{
  char message[1024];

  size_t size = message_input(message, sizeof(message));

  return message_hash_create(hash, message, size);
}

/*
 * Create hash from stdin message, and print it
 */
static int stdin_hash_print(void)
{
  char hash[64 + 1];
  memset(hash, '\0', sizeof(hash));

  if (stdin_hash_create(hash) == 0)
  {
    printf("hashing: Failed to create stdin hash\n");
    return 1;
  }

  printf("%s  -\n", hash);

  return 0;
}

static struct argp argp = { options, opt_parse, args_doc, doc };

/*
 * This is the main function
 */
int main(int argc, char* argv[])
{
  argp_parse(&argp, argc, argv, 0, 0, &args);

  // If no paths was supplied, input from stdin
  if (args.path_count == 0)
  {
    stdin_hash_print();
  }
  else if (args.concat)
  {
    char** files = NULL;
    size_t count = 0;

    for (size_t index = 0; index < args.path_count; index++)
    {
      char* path = args.paths[index];

      if (files_get(&files, &count, path, args.depth) == 0)
      {
        printf("hashing: %s: No file or directory\n", path);
      }
    }

    files_hash_print(files, count);

    files_free(files, count);
  }
  else
  {
    for (size_t index = 0; index < args.path_count; index++)
    {
      char* path = args.paths[index];

      if (strcmp(path, "-") == 0)
      {
        stdin_hash_print();
        continue;
      }

      char** files = NULL;
      size_t count = 0;

      if (files_get(&files, &count, path, args.depth) == 0)
      {
        printf("hashing: %s: No file or directory\n", path);
        continue;
      }

      file_hashes_print(files, count);

      files_free(files, count);
    }
  }

  if (args.paths) free(args.paths);

  return 0;
}
