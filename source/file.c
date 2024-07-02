/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-07-01
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
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

  if(!stream) return 0;

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

  if(!stream) return 0;

  int status = fread(pointer, size, nmemb, stream);

  fclose(stream);

  return status;
}

/*
 * Concatonate directory base path and name to get full path
 *
 * PARAMS
 * - const char* base | Directory path
 * - const char* name | Name of file
 *
 * RETURN (char* fullpath)
 */
static char* full_path(const char* base, const char* name)
{
  size_t bsize = strlen(base);
  size_t nsize = strlen(name);

  char* fullpath = malloc(sizeof(char) * (bsize + nsize + 2));

  sprintf(fullpath, "%s/%s", base, name);

  return fullpath;
}

/*
 * PARAMS
 * - char**      files   | The files in the directory
 * - size_t*     count   | Number of files
 * - const char* dirpath | The path to the directory
 * - int         depth   | How many directories to open
 * 
 * Note:
 * If the depth is -1, search indefinitely
 * If the depth is  0, stop searching
 *
 * RETURN (int status)
 * - 0 | Success!
 * - 1 | Failed to open directory
 */
static int dir_files(char** files, size_t* count, const char* dirpath, int depth)
{
  if(depth == 0) return 0;

  struct dirent* dire;

  DIR* dirp = opendir(dirpath);

  if(!dirp) return 1;

  while((dire = readdir(dirp)) != NULL)
  {
    if(strcmp(dire->d_name, ".")  == 0) continue;

    if(strcmp(dire->d_name, "..") == 0) continue;


    if(dire->d_type == DT_DIR)
    {
      char* fullpath = full_path(dirpath, dire->d_name);

      if(depth == -1)
      {
        dir_files(files, count, fullpath, -1);
      }
      else dir_files(files, count, fullpath, depth - 1);

      free(fullpath);
    }
    else if(dire->d_type == DT_REG)
    {
      files[(*count)++] = full_path(dirpath, dire->d_name);
    }
  }

  closedir(dirp);

  return 0; // Success!
}

/*
 * Get the count of files a directory contain recursivly
 *
 * PARAMS
 * - const char* dirpath | The directory to count files in
 * - int         depth   | How many directories to open
 *
 * RETURN (size_t count)
 */
static size_t dir_count(const char* dirpath, int depth)
{
  if(depth == 0) return 0;

  struct dirent* dire;

  DIR* dirp = opendir(dirpath);

  if(!dirp) return 0;

  int count = 0;

  while((dire = readdir(dirp)) != NULL)
  {
    if(strcmp(dire->d_name, ".")  == 0) continue;

    if(strcmp(dire->d_name, "..") == 0) continue;


    if(dire->d_type == DT_DIR)
    {
      char* fullpath = full_path(dirpath, dire->d_name);

      if(depth == -1)
      {
        count += dir_count(fullpath, -1);
      }
      else count += dir_count(fullpath, depth - 1);

      free(fullpath);
    }
    else if(dire->d_type == DT_REG) count++;
  }

  closedir(dirp);

  return count;
}

/*
 * Determine the type of path, either dir or file
 *
 * PARAMS
 * - const char* path | The path to either a dir or a file
 *
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
 * Calculate the number of files in path recursivly
 *
 * PARAMS
 * - const char* path  | The path to either a dir or a file
 * - int         depth | How many directories to open
 *
 * RETURN (size_t count)
 */
static size_t paths_count(const char* path, int depth)
{
  if(depth == 0) return 0;

  switch(path_type(path))
  {
    case 1: return 1;

    case 2: return dir_count(path, depth);

    default: return 0;
  }
}

/*
 * PARAMS
 * - char** paths      |
 * - size_t path_count |
 * - int    depth      | How many directories to open
 *
 * RETURN (size_t count)
 */
size_t files_count(char** paths, size_t path_count, int depth)
{
  size_t count = 0;

  for(size_t index = 0; index < path_count; index++)
  {
    if(strcmp(paths[index], "-") == 0)
    {
      count++;
    }
    else count += paths_count(paths[index], depth);
  }

  return count;
}

/*
 * PARAMS
 * - char**      files |
 * - size_t*     count |
 * - const char* path  |
 * - int         depth |
 *
 * RETURN (int status)
 * - 0 | Error
 * - 1 | Parsed path as file
 * - 2 | Parsed path as dir
 */
static int path_files(char** files, size_t* count, const char* path, int depth)
{
  switch(path_type(path))
  {
    case 1:
      files[(*count)++] = strdup(path);
      return 1;

    case 2:
      dir_files(files, count, path, depth);
      return 2;

    default:
      return 0;
  }
}

/*
 * PARAMS
 * - size_t count      |
 * - char** paths      |
 * - size_t path_count |
 * - int    depth      |
 *
 * RETURN (char** files)
 */
char** files_create(size_t count, char** paths, size_t path_count, int depth)
{
  if(count == 0) return NULL;

  char** files = malloc(sizeof(char*) * count);

  size_t file_index = 0;

  for(int index = 0; index < path_count; index++)
  {
    if(strcmp(paths[index], "-") == 0)
    {
      files[file_index++] = strdup(paths[index]);
    }
    else path_files(files, &file_index, paths[index], depth);
  }
  
  return files;
}

/*
 * PARAMS
 * - char** files |
 * - size_t count |
 *
 * RETURN (size_t size)
 */
size_t files_size(char** files, size_t count)
{
  size_t size = 0;

  for(size_t index = 0; index < count; index++)
  {
    size += file_size(files[index]);
  }

  return size;
}

/*
 * Read and concatonate multiple files into single memory at pointer
 *
 * PARAMS
 * - void*  pointer |
 * - size_t size    |
 * - size_t nmemb   |
 * - char** files   |
 * - size_t count   |
 *
 * RETURN (int status)
 * - 0 | Success!
 * - 1 | Failed to read a file
 */
int files_read(void* pointer, size_t size, size_t nmemb, char** files, size_t count)
{
  size_t shift = 0;

  for(int index = 0; index < count; index++)
  {
    size_t fsize = file_size(files[index]);

    if(fsize == 0) continue;

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
 * - char** files |
 * - size_t count |
 */
void files_free(char** files, size_t count)
{
  if(!files) return;

  for(int index = 0; index < count; index++)
  {
    if(files[index]) free(files[index]);
  }

  free(files);
}
