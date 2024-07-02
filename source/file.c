/*
 * Read data from files
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-07-02
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

/*
 * Get the size of the file at the inputted path
 *
 * The function returns the number of bytes the file contains
 *
 * PARAMS
 * - const char* filepath | Path to file
 *
 * RETURN (size_t size)
 * - 0  | Error
 * - >0 | Success
 */
size_t file_size_get(const char* filepath)
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
 * Read a number of chunks from file to memory at pointer
 *
 * The function returns the number of read chunks
 *
 * PARAMS
 * - void*       pointer  | Pointer to memory to read data to
 * - size_t      msize    | Size of each member
 * - size_t      nmemb    | Number of members
 * - const char* filepath | Path to file
 *
 * RETURN (same as fread, size_t read_nmemb)
 * - 0  | Error
 * - >0 | Success
 */
size_t file_read(void* pointer, size_t msize, size_t nmemb, const char* filepath)
{
  if(!pointer) return 0;

  FILE* stream = fopen(filepath, "rb");

  if(!stream) return 0;

  size_t read_nmemb = fread(pointer, msize, nmemb, stream);

  fclose(stream);

  return read_nmemb;
}

/*
 * Concatonate directory base path and name to get full path
 *
 * PARAMS
 * - const char* base | Directory path
 * - const char* name | Name of file
 *
 * EXPECT
 * - both base and name are allocated
 *
 * RETURN (char* fullpath)
 */
static char* full_path_create(const char* base, const char* name)
{
  size_t bsize = strlen(base);
  size_t nsize = strlen(name);

  char* fullpath = malloc(sizeof(char) * (bsize + nsize + 2));

  sprintf(fullpath, "%s/%s", base, name);

  return fullpath;
}

/*
 * Allocate file paths from directory to array of filepaths
 *
 * PARAMS
 * - char***     files   | Pointer to paths to files
 * - size_t*     count   | Number of files
 * - const char* dirpath | Path to directory
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
static int dir_files_alloc(char*** files, size_t* count, const char* dirpath, int depth)
{
  if(depth == 0) return 0;

  struct dirent* dire;

  DIR* dirp = opendir(dirpath);

  if(!dirp) return 1;

  while((dire = readdir(dirp)) != NULL)
  {
    if(strcmp(dire->d_name, ".")  == 0) continue;

    if(strcmp(dire->d_name, "..") == 0) continue;


    if(dire->d_type == DT_REG)
    {
      *files = realloc(*files, sizeof(char*) * (*count + 1));

      (*files)[(*count)++] = full_path_create(dirpath, dire->d_name);
    }
    else if(dire->d_type == DT_DIR)
    {
      char* fullpath = full_path_create(dirpath, dire->d_name);

      if(depth == -1)
      {
        dir_files_alloc(files, count, fullpath, -1);
      }
      else dir_files_alloc(files, count, fullpath, depth - 1);

      free(fullpath);
    }
  }

  closedir(dirp);

  return 0; // Success!
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
static int path_type_get(const char* path)
{
  struct stat pstat;

  if(stat(path, &pstat) == -1) return 0;

  if(pstat.st_mode & S_IFREG) return 1;

  if(pstat.st_mode & S_IFDIR) return 2;

  return 0;
}

/*
 * Allocate either path to file or paths to files in directory
 *
 * PARAMS
 * - char***     files | Pointer to paths to files
 * - size_t*     count | Number of files
 * - const char* path  | Path to either file or dir
 * - int         depth | How many directories to open
 *
 * EXPECT
 * - files, count and path are allocated
 */
static void path_files_alloc(char*** files, size_t* count, const char* path, int depth)
{
  int path_type = path_type_get(path);

  // 1. If the path points to a file, or is blank
  if(path_type == 1 || strcmp(path, "-") == 0)
  {
    *files = realloc(*files, sizeof(char*) * (*count + 1));

    (*files)[(*count)++] = strdup(path);
  }
  else if(path_type == 2)
  {
    dir_files_alloc(files, count, path, depth);
  }
}

/*
 * Create an array of paths to files based on inputted paths to either file or dir
 *
 * Last updated: 2024-07-02
 *
 * PARAMS
 * - size_t* count      | Pointer to number of files
 * - char**  paths      | Paths to either file or dir
 * - size_t  path_count | Number of paths to either file or dir
 * - int     depth      | How many directories to open
 *
 * RETURN (char** files)
 */
char** files_create(size_t* count, char** paths, size_t path_count, int depth)
{
  if(!count || !paths) return NULL;

  char** files = NULL;
  *count = 0;

  for(size_t index = 0; index < path_count; index++)
  {
    path_files_alloc(&files, count, paths[index], depth);
  }
  
  return files;
}

/*
 * Get the combined size of data of the inputted files
 *
 * PARAMS
 * - char** files | Paths to files
 * - size_t count | Number of files
 *
 * RETURN (size_t size)
 */
size_t files_size_get(char** files, size_t count)
{
  if(!files) return 0;

  size_t size = 0;

  for(size_t index = 0; index < count; index++)
  {
    size += file_size_get(files[index]);
  }

  return size;
}

/*
 * Read and concatonate multiple files into single memory at pointer
 *
 * The function returns the total number of read chunks
 *
 * If the files contain more data than the inputted size can hold, no more data will be read
 *
 * Note: The msize can not be 0, then division by 0 would occur
 *
 * PARAMS
 * - void*  pointer | Pointer to memory to save read data to
 * - size_t msize   | Size of each member
 * - size_t nmemb   | Number of members
 * - char** files   | Paths to files
 * - size_t count   | Number of files
 *
 * RETURN (size_t read_nmemb)
 */
size_t files_read(void* pointer, size_t msize, size_t nmemb, char** files, size_t count)
{
  if(!pointer || msize == 0 || !files) return 0;

  size_t read_nmemb = 0;

  for(size_t index = 0; index < count; index++)
  {
    size_t file_size = file_size_get(files[index]);

    size_t file_nmemb = file_size / msize;

    // If the total read data would overwrite the buffer,
    // only read the amount of data that the buffer can hold
    if(read_nmemb + file_nmemb > nmemb)
    {
      read_nmemb += file_read(pointer + read_nmemb * msize, msize, nmemb - read_nmemb, files[index]);
      
      break;
    }

    read_nmemb += file_read(pointer + read_nmemb * msize, msize, file_nmemb, files[index]);
  }

  return read_nmemb;
}

/*
 * Free the pointers in the files string array
 *
 * PARAMS
 * - char** files | Pointer to strings (pointer to char)
 * - size_t count | Number of pointers
 */
void files_free(char** files, size_t count)
{
  if(!files) return;

  for(size_t index = 0; index < count; index++)
  {
    if(files[index]) free(files[index]);
  }

  free(files);
}
