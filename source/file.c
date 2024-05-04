/*
 * Written by Hampus Fridholm
 *
 * 2024-05-03
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>

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
 * Concatonate directory base path and name to get full path
 *
 * PARAMS
 * - const char* base |
 * - const char* name |
 *
 * RETURN (char* fullpath)
 */
char* full_path(const char* base, const char* name)
{
  size_t bsize = strlen(base);
  size_t nsize = strlen(name);

  char* fullpath = malloc(bsize + nsize + 2);

  sprintf(fullpath, "%s/%s", base, name);

  return fullpath;
}

/*
 * PARAMS
 * - char**      files   |
 * - size_t*     amount  |
 * - const char* dirpath |
 * - int         depth   |
 * 
 * Note:
 * If the depth is -1, search indefinitely
 * If the depth is  0, stop searching
 *
 * RETURN (int status)
 * - 0 | Success!
 * - 1 | Failed to open directory
 */
int dir_files(char** files, size_t* amount, const char* dirpath, int depth)
{
  if(depth == 0) return 0;

  struct dirent* dire;

  DIR* dirp = opendir(dirpath);

  if(dirp == NULL) return 1;

  while((dire = readdir(dirp)) != NULL)
  {
    if(!strcmp(dire->d_name, ".")) continue;

    if(!strcmp(dire->d_name, "..")) continue;


    if(dire->d_type == DT_DIR)
    {
      char* fullpath = full_path(dirpath, dire->d_name);

      if(depth == -1)
      {
        dir_files(files, amount, fullpath, -1);
      }
      else
      {
        dir_files(files, amount, fullpath, depth - 1);
      }

      free(fullpath);
    }
    else if(dire->d_type == DT_REG)
    {
      files[(*amount)++] = full_path(dirpath, dire->d_name);
    }
  }

  closedir(dirp);

  return 0; // Success!
}

/*
 * Get the amount of files a directory contain recursivly
 *
 * PARAMS
 * - const char* dirpath | The directory to count files in
 *
 * RETURN (size_t amount)
 */
size_t dir_amount(const char* dirpath, int depth)
{
  if(depth == 0) return 0;

  struct dirent* dire;

  DIR* dirp = opendir(dirpath);

  if(dirp == NULL) return 0;

  int amount = 0;

  while((dire = readdir(dirp)) != NULL)
  {
    if(!strcmp(dire->d_name, ".")) continue;

    if(!strcmp(dire->d_name, "..")) continue;


    if(dire->d_type == DT_DIR)
    {
      char* fullpath = full_path(dirpath, dire->d_name);

      if(depth == -1)
      {
        amount += dir_amount(fullpath, -1);
      }
      else
      {
        amount += dir_amount(fullpath, depth - 1);
      }

      free(fullpath);
    }
    else if(dire->d_type == DT_REG) amount++;
  }

  closedir(dirp);

  return amount;
}

/*
 * RETURN (int type)
 * - 0 | Path does not exist
 * - 1 | File
 * - 2 | Directory
 */
static int path_type(const char* path)
{
  struct stat pstat;

  stat(path, &pstat);

  if(pstat.st_mode & S_IFREG) return 1;

  if(pstat.st_mode & S_IFDIR) return 2;

  return 0;
}

/*
 *
 */
size_t path_amount(const char* path, int depth)
{
  if(depth == 0) return 0;

  switch(path_type(path))
  {
    case 1: return 1;

    case 2: return dir_amount(path, depth);

    default: return 0;
  }
}

/*
 *
 */
size_t files_amount(char** paths, size_t pamount, int depth)
{
  size_t amount = 0;

  for(size_t index = 0; index < pamount; index++)
  {
    amount += path_amount(paths[index], depth);
  }

  return amount;
}

/*
 * PARAMS
 *
 * RETURN (char* dup)
 */
char* string_dup(const char* string)
{
  char* dup = malloc(strlen(string) + 1);

  sprintf(dup, "%s", string);
  
  return dup;
}

/*
 * RETURN (int status)
 * - 0 | Error
 * - 1 | Parsed path as file
 * - 2 | Parsed path as dir
 */
int path_files(char** files, size_t* amount, const char* path, int depth)
{
  switch(path_type(path))
  {
    case 1:
      files[(*amount)++] = string_dup(path);
      return 1;

    case 2:
      dir_files(files, amount, path, depth);
      return 2;

    default:
      return 0;
  }
}

/*
 *
 */
char** files_create(size_t amount, char** paths, size_t pamount, int depth)
{
  if(amount == 0) return NULL;

  char** files = malloc(sizeof(char*) * amount);

  size_t findex = 0;

  for(int index = 0; index < pamount; index++)
  {
    path_files(files, &findex, paths[index], depth);
  }
  
  return files;
}

/*
 *
 */
size_t files_size(char** files, size_t amount)
{
  size_t size = 0;

  for(size_t index = 0; index < amount; index++)
  {
    size += file_size(files[index]);
  }

  return size;
}

/*
 * Read and concatonate multiple files into single memory at pointer
 *
 * PARAMS
 *
 * RETURN (int status)
 */
int files_read(void* pointer, size_t size, size_t nmemb, char** files, size_t amount)
{
  size_t shift = 0;

  for(int index = 0; index < amount; index++)
  {
    size_t fsize = file_size(files[index]);

    printf("%ld : %s\n", fsize, files[index]);

    fsize = (shift + fsize > size) ? (size - shift) : fsize;

    int status = file_read(pointer + shift, fsize, nmemb, files[index]);

    if(status == 0) return 1;

    shift += fsize;
  }
  return 0;
}

/*
 * Free the pointers in the files string array
 *
 * PARAMS
 * - char** files  |
 * - size_t amount |
 */
void files_free(char** files, size_t amount)
{
  for(int index = 0; index < amount; index++)
  {
    free(files[index]);
  }
  free(files);
}
