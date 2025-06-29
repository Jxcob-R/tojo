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
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
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

#define _DIR_NEXT_ID_F "NEXT_ID"    /* Next available item ID */
#define _DIR_CODE_LIST_F "LISTED_CODES" /* Codes listed in previous list */

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
(   /* Item ID */ \
    HEX_LEN(sitem_id) + _DIR_ITEM_FIELD_DELIM_LEN \
    + /* Item character code */ \
    ITEM_CODE_LEN + _DIR_ITEM_FIELD_DELIM_LEN \
    + /* Item name */ \
    ITEM_NAME_MAX + _DIR_ITEM_DELIM_LEN \
)

/* Other macros */
#define OFF_T_MIN ((off_t)(((off_t)1) << (sizeof(off_t) * 8 - 1)))

/**
 * @brief Check if directory is a current project
 * @param dir Write relative project  directory to dir if it exists, leave
 * empty otherwise.
 * otherwise.
 * @return 0 on success, some error value otherwise
 * @note Must be called before any other dir* function due to static memory
 * usage
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
extern void dir_construct_path(const char * const path, const char *base,
                               char *buf, const size_t max);

/**
 * @brief Initialise project directory at path
 * @param path Path to initialise project at, path must be a relative path
 * @return 0 on success, -1 otherwise
 * @see mkdir
 */
extern int dir_init(const char *path);

/**
 * @brief Count the total number of items added to the project, regardless of
 * status
 * @return Number of items
 * @return Negative value in case of error
 */
extern int dir_total_items(void);

/**
 * @brief Get next available item ID, this call also changes the next available
 * ID so it is assumed that the ID retrieved is then used for an new item
 * @return Next available item ID for project
 * @return If called on new project, ID of -1 is returned, subsequent calls will
 * index from 0
 * @return -2 on error
 */
extern sitem_id dir_next_id(void);

/**
 * @brief Read items of a single given status
 * @param st Status of items to read
 * @return NULL-terminated array of item pointers allocated on the heap
 */
extern item ** dir_read_items_status(enum status st);

/**
 * @brief Read all items in project at path
 * @return Pointer to an array of items allocated on the heap with a
 * terminating all zero/NULL item
 * @note Function *only* extracts names as of now which are assumed to be
 * separated by some _DIR_ITEM_DELIM
 */
extern item ** dir_read_all_items(void);

/**
 * @brief Append write the item it to the project.
 * @param it Pointer to item to write
 * @return 0 on success
 * @return -1 on error
 */
extern int dir_append_item(const item *it);

/**
 * @brief Change status of item given its ID
 * @param id ID of item to change
 * @param new_status Status to change item to
 * @return 0 if item status change was succesful
 * @return -1 if item status could not be changed
 */
extern int dir_change_item_status_id(const sitem_id id,
                                     const enum status new_status);

/**
* @brief Store item codes of items in project
* @param items List of item pointers, terminated by a NULL pointer
* @param prefix_lengths List of unique prefix lengths of codes, corresponding to
* elements in items
* @see dir_get_id_from_prefix
*/
extern void dir_write_item_codes(item *const *items,
                                 const int *prefix_lengths);

/**
 * @brief Return the ID of the item associated with the listed code prefix
 * @return ID of associated item
 * @return -1 if no items have been listed in this project or code_prefix is
 * NULL
 * @note Not suitable for full (ITEM_CODE_LEN) codes, only listed *prefixes*
 * (which may technically be ITEM_CODE_LEN characters long)
 * @see dir_write_item_codes
 */
extern sitem_id dir_get_id_from_prefix(const char *code_prefix);

/**
 * @brief Retrieve the item in the project with the given code
 * @param full_code Full ITEM_CODE_LEN code (may or may not be null terminated).
 * @return Heap-allocated pointer to item with associated code
 */
extern item * dir_get_item_with_code(const char *full_code);
#endif
