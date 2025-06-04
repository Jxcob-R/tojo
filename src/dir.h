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
#define _DIR_ITEM_PATHF "items"

#define _DIR_ITEM_DELIM "\n" /* Item data delimiter - note char * type */

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
 * @param path Project path
 * @return Number of items, negative value indicates some error
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
 */
extern int dir_write_item(const item *it, const char *path);

#endif
