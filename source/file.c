/*
 * Read data from files
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-09-13
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

/*
 * Get the size of the file at the supplied path
 *
 * The function returns the number of bytes in the file
 *
 * PARAMS
 * - const char* filepath | Path to file
 *
 * RETURN (size_t size)
 * - 0  | Failed to open file, or file is empty
 * - >0 | Number of bytes in file
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
 * Read a number of bytes from file to memory at pointer
 *
 * The function returns the number of read bytes
 *
 * PARAMS
 * - void*       pointer  | Pointer to memory buffer
 * - size_t      size     | Number of bytes to read
 * - const char* filepath | Path to file
 *
 * RETURN (same as fread, size_t read_size)
 * - 0  | Failed to read file, or bad input
 * - >0 | Number of read bytes
 */
size_t file_read(void* pointer, size_t size, const char* filepath)
{
  if(!pointer) return 0;

  FILE* stream = fopen(filepath, "rb");

  if(!stream) return 0;

  size_t read_size = fread(pointer, 1, size, stream);

  fclose(stream);

  return read_size;
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
 *
 * Note: Remember to free the allocated memory
 */
static char* full_path_create(const char* base, const char* name)
{
  size_t base_size = strlen(base);
  size_t name_size = strlen(name);

  char* fullpath = malloc(sizeof(char) * (base_size + name_size + 2));

  sprintf(fullpath, "%s/%s", base, name);

  return fullpath;
}

static void dir_files_alloc(char*** files, size_t* count, const char* dirpath, int depth);

/*
 * Allocate files in directory child - either a file or a new directory
 *
 * PARAMS
 * - char***     files      | Filepaths
 * - size_t*     count      | Number of files
 * - const char* dirpath    | Path to directory
 * - int         child_type | Type of child - file or dir
 * - const char* child_name | Name of child - filename or dirname
 * - int         depth      | Search depth limit
 */
static void dir_child_files_alloc(char*** files, size_t* count, const char* dirpath, int child_type, const char* child_name, int depth)
{
  if(child_type == DT_REG)
  {
    *files = realloc(*files, sizeof(char*) * (*count + 1));

    (*files)[(*count)++] = full_path_create(dirpath, child_name);
  }
  else if(child_type == DT_DIR)
  {
    char* fullpath = full_path_create(dirpath, child_name);

    if(depth == -1)
    {
      dir_files_alloc(files, count, fullpath, -1);
    }
    else
    {
      dir_files_alloc(files, count, fullpath, depth - 1);
    }

    free(fullpath);
  }
}

/*
 * Allocate file paths from directory to array of filepaths
 *
 * PARAMS
 * - char***     files   | Pointer to filepaths
 * - size_t*     count   | Number of files
 * - const char* dirpath | Path to directory
 * - int         depth   | Search depth limit
 * 
 * Note:
 * If the depth is -1, search indefinitely
 * If the depth is  0, stop searching
 */
static void dir_files_alloc(char*** files, size_t* count, const char* dirpath, int depth)
{
  if(depth == 0) return;

  struct dirent* dire;

  DIR* dirp = opendir(dirpath);

  if(!dirp) return;

  while((dire = readdir(dirp)) != NULL)
  {
    if(strcmp(dire->d_name, ".")  == 0) continue;

    if(strcmp(dire->d_name, "..") == 0) continue;

    dir_child_files_alloc(files, count, dirpath, dire->d_type, dire->d_name, depth);
  }

  closedir(dirp);
}

/*
 * Determine the type of path, either dir or file
 *
 * PARAMS
 * - const char* path | Path to either file or dir
 *
 * RETURN (int type)
 * - 0 | Path doesn't exist
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
 * - char***     files | Pointer to filepaths
 * - size_t*     count | Number of files
 * - const char* path  | Path to either file or dir
 * - int         depth | Search depth limit
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
  else
  {
    printf("hashing: %s: No such file or directory\n", path);
  }
}

/*
 * Create an array of paths to files based on inputted paths to either file or dir
 *
 * PARAMS
 * - size_t* count      | Pointer to number of files
 * - char**  paths      | Paths to either file or dir
 * - size_t  path_count | Number of paths
 * - int     depth      | Search depth limit
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
 * - char** files | Filepaths
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
 * The function returns the total number of read bytes
 *
 * If the total read data would overwrite the buffer,
 * only read the amount of data that the buffer can hold
 *
 * PARAMS
 * - void*  pointer | Pointer to memory buffer
 * - size_t size    | Number of bytes to read
 * - char** files   | Filepaths
 * - size_t count   | Number of files
 *
 * RETURN (size_t read_size)
 */
size_t files_read(void* pointer, size_t size, char** files, size_t count)
{
  if(!pointer || !files) return 0;

  size_t read_size = 0;

  for(size_t index = 0; index < count; index++)
  {
    size_t file_size = file_size_get(files[index]);

    // If the current file would fill up the buffer,
    // read the last available bytes and return
    if(read_size + file_size > size)
    {
      read_size += file_read(pointer + read_size, size - read_size, files[index]);
      
      break;
    }
    else
    {
      read_size += file_read(pointer + read_size, file_size, files[index]);
    }
  }

  return read_size;
}

/*
 * Free the pointers in the files string array
 *
 * PARAMS
 * - char** files | Filepaths
 * - size_t count | Number of files
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
