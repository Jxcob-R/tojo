/**
 * @brief Directory module for interfacing with project data directory
 * 
 * A plain-text data writing protocol has been implemented in release 0.1,
 * which may be subject to future change.
 */
#ifndef DIR_H
#define DIR_H

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ds/item.h"

/*
 * Project directory substructure
 * This should be considered only internally and not part of the dir interface
 */
#define _DIR_ITEM_PATH_D "items"    /* Items directory */
#define _DIR_ITEM_TODO_F "todo"     /* Items staged for completion */
#define _DIR_ITEM_INPROG_F "ip"     /* Items currently mared as in progress */
#define _DIR_ITEM_DONE_F "done"     /* Complete items */

#define _DIR_ITEM_NUM_FILES 3 /* Number of item files categorised */

#define _DIR_ITEM_DELIM "\n"        /* Item delimiter - note char * type */
#define _DIR_ITEM_FIELD_DELIM ":"   /* Item field delimiter */

/**
 * @brief Check if directory is a current project
 * @param dir Write relative project  directory to dir if it exists, leave
 * empty otherwise.
 * otherwise.
 * @return 0 on success, some error value otherwise
 */
int dir_find_project(char *dir);

/**
 * @brief Construct path from a directory path and a base and copy into buf
 * (with a maximum of max bytes)
 * @param path NULL-terminated path name
 * @param base NULL-terminated name of base file or directory
 * @param buf Buffer to write full path to
 * @param max Maximum bytes to write to buffer
 * @note Does not check if the base is a member of the directory provided
 * by path, nor are any other existence checks made
 */
extern void dir_construct_path(const char *path, const char *base, char *buf,
                               const size_t max);

/**
 * @brief Initialise project directory at path
 * @param path Path to initialise project at, path must be a relative path
 * @return 0 on success, -1 otherwise
 * @see mkdir
 */
extern int dir_init(const char *path);

/**
 * @brief Find item with name in project
 * @param name Name of item to find
 * @param path Path to the project
 * @return Pointer to an item allocated on the heap, NULL if the item does not
 * exist
 */
extern item * dir_find_item(const char *name, const char *path);

/**
 * @brief Count all todo items open in project, separated by _DIR_ITEM_DELIM
 * @param path Project path, may be NULL if the internal item_path is already
 * known as a result of a previous dir_* function call that reads/writes to
 * item path
 * @return Number of items
 * @return Negative value in case of error
 */
extern int dir_number_of_items(const char *path);

/**
 * @brief Get all items in project at path
 * @return Pointer to an array of items allocated on the heap with a
 * terminating all zero/NULL item
 * @note Function *only* extracts names as of now which are assumed to be
 * separated by some _DIR_ITEM_DELIM
 */
extern item ** dir_get_all_items(const char *path);

/**
 * @brief Write the item it to the project.
 * @param it Pointer to item to write
 * @param path Path to the project
 * @return 0 on success
 * @return -1 on error
 */
extern int dir_write_item(const item *it, const char *path);

#endif
