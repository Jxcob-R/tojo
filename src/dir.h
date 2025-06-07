/**
 * @brief Directory module for interfacing with project data directory
 * 
 * A plain-text data writing protocol has been implemented in release 0.1,
 * which may be subject to future change.
 *
 * The project path is a parameter of a large portion of these functions,
 * however, the calling routine may specify these as NULL if a dir_* function
 * has already been called with the correct path due to expectation that dir
 * will save the state of the project in a single runtime.
 */
#ifndef DIR_H
#define DIR_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* Used for syncfs file synchronisation */
#endif

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
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

/**
 * Special characters/tokens (for item entry)
 * @note _LEN = strlen(...) or equivalent
 * @note _SZ = sizeof(...) value
 */

#define _DIR_ITEM_DELIM "\n"        /* Item delimiter - note char * type */
#define _DIR_ITEM_DELIM_LEN (sizeof(_DIR_ITEM_DELIM) - 1)

#define _DIR_ITEM_FIELD_DELIM ":" /* Item field delimiter */
#define _DIR_ITEM_FIELD_DELIM_LEN (sizeof(_DIR_ITEM_FIELD_DELIM) - 1)

/* Item entry format */
#define DIR_ITEM_ENTRY_LEN \
    ( /* Item ID */ \
    sizeof(ssize_t)*2 + _DIR_ITEM_FIELD_DELIM_LEN \
    + /* Item name */ \
    ITEM_NAME_MAX + _DIR_ITEM_DELIM_LEN \
)

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
extern void dir_construct_path(const char * const path, const char *base, char *buf,
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
 * @brief Count the total number of items added to the project, regardless of
 * status
 * @param path Project path, may be NULL if the internal item_path is already
 * known as a result of a previous dir_* function call that reads/writes to
 * item path
 * @return Number of items
 * @return Negative value in case of error
 */
extern int dir_total_items(const char *path);

/**
 * @brief Read items of a single given status
 * @param path Project path
 * @param st Status of items to read
 * @return NULL-terminated array of item pointers allocated on the heap
 */
extern item ** dir_read_items_status(const char *const path, enum status st);

/**
 * @brief Read all items in project at path
 * @return Pointer to an array of items allocated on the heap with a
 * terminating all zero/NULL item
 * @note Function *only* extracts names as of now which are assumed to be
 * separated by some _DIR_ITEM_DELIM
 */
extern item ** dir_read_all_items(const char * const path);

/**
 * @brief Append write the item it to the project.
 * @param it Pointer to item to write
 * @param path Path to the project
 * @return 0 on success
 * @return -1 on error
 */
extern int dir_append_item(const item *it, const char *path);

#endif
