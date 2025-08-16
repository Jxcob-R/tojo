#include "dir.h"
#include "dev-utils/test-helpers.h"
#include "ds/graph.h"
#include "ds/item.h"
#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Project directory */
static char proj_path[MAX_PATH] = {'\0'};

/* Item storage */
static char items_path[MAX_PATH] = {'\0'};
static char backlog_path[MAX_PATH] = {'\0'};
static char todo_path[MAX_PATH] = {'\0'};
static char ip_path[MAX_PATH] = {'\0'};
static char done_path[MAX_PATH] = {'\0'};

static char next_id_path[MAX_PATH] = {'\0'};      /* Next available item ID */
static char listed_codes_path[MAX_PATH] = {'\0'}; /* Listed codes */
static char item_dependencies[MAX_PATH] = {'\0'}; /* Item dependencies */

/**
 * @brief Set up global path variables to default values if not already done
 * so. Should be one of the first calls made by dir_* functions (excluding) for
 * project initialisation.
 * @param path Path to project directory may be NULL
 * @note Modifies global variables, should be used once in a dir API call
 * @note Calling with NULL assumes that project path is already known
 */
static_fn void setup_path_names(const char *const path) {
    if (!*proj_path && path)
        strcpy(proj_path, path);

    /* Set up items path */
    if (!*items_path)
        dir_construct_path(proj_path, _DIR_ITEM_PATH_D, items_path, MAX_PATH);

    /* Item file names */
    if (!*backlog_path)
        dir_construct_path(items_path, _DIR_ITEM_BACKLOG_F, backlog_path,
                           MAX_PATH);
    if (!*todo_path)
        dir_construct_path(items_path, _DIR_ITEM_TODO_F, todo_path, MAX_PATH);
    if (!*ip_path)
        dir_construct_path(items_path, _DIR_ITEM_INPROG_F, ip_path, MAX_PATH);
    if (!*done_path)
        dir_construct_path(items_path, _DIR_ITEM_DONE_F, done_path, MAX_PATH);

    /* Next ID data */
    if (!*next_id_path)
        dir_construct_path(proj_path, _DIR_NEXT_ID_F, next_id_path, MAX_PATH);
    /* Listed codes available */
    if (!*listed_codes_path)
        dir_construct_path(proj_path, _DIR_CODE_LIST_F, listed_codes_path,
                           MAX_PATH);
    /* Item dependencies */
    if (!*item_dependencies)
        dir_construct_path(proj_path, _DIR_DEPENDENICES_F, item_dependencies,
                           MAX_PATH);
}

/**
 * @brief Create a file given by fname; used for project files.
 * @param fname Name of file to create
 * @return 0 for successful file creation
 * @return 1 in case of EEXIST
 * @return 2 in case of other error
 */
static_fn int create_file(const char *const fname) {
    int fd = open(fname, O_CREAT, CONF_DIR_PERMS & 0666);
    if (fd < 0) {
        return 1 + (errno != EEXIST);
    }
    close(fd);
    return 0;
}

/**
 * @brief creates items file, file descriptor is closed immediately if creation
 * is successful
 * @return 0 on success
 * @return -1 on error
 * @see open, close, mkdir
 */
static_fn int create_items() {
    setup_path_names(NULL);

    /* Create directory */
    int ret = mkdir(items_path, CONF_DIR_PERMS);

    if (ret == -1) {
#ifdef DEBUG
        log_err("Could not create items directory");
#endif
        /*
         * Function should only ever be called on new projects
         * This might have other consequences down the road that I, frankly,
         * don't want to think about right now
         */
        assert(errno != EEXIST);
        return -1;
    }

    /* Open item storage files */
    int file_creation = (create_file(backlog_path) + create_file(todo_path) +
                         create_file(ip_path) + create_file(done_path));

    if (file_creation != 0) {
#ifdef DEBUG
        log_err("Could not create item storage files");
#endif
        return -1;
    }

    return 0;
}

/**
 * @brief Open all item files in items directory and populate item_fds array.
 * @param flags open function flags
 * @param item_fds Array of file descriptors to write open item file
 * descriptors to
 * @note Can only support 'simple' flags that do not require extra arguments,
 * i.e. O_WRONLY or O_RDWR
 * @note Errors with open are not handled
 * @see open
 * @see close_items
 */
static_fn void open_items(const int flags, int item_fds[_DIR_ITEM_NUM_FILES]) {
    assert(strlen(items_path) >=
           sizeof(CONF_PROJ_DIR) + sizeof(_DIR_ITEM_PATH_D) - 1);

    if (!item_fds)
        return;

    item_fds[0] = open(backlog_path, flags);
    item_fds[1] = open(todo_path, flags);
    item_fds[2] = open(ip_path, flags);
    item_fds[3] = open(done_path, flags);

    if (errno == ENOTDIR) {
        puts("No project listed");
#ifdef DEBUG
        log_err("open_items called with incorrect path");
#endif
    }
}

/**
 * @brief Close all item file descriptors opened by open_items
 * @param item_fds Array of file descriptors of item files
 * @note Errors with close are not handled
 * @see open
 * @see open_items
 */
static_fn void close_items(const int item_fds[_DIR_ITEM_NUM_FILES]) {
    if (!item_fds)
        return;

    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        close(item_fds[i]);
    }
}

/**
 * @brief Open file descriptor associated with the items with status st, using
 * flags
 * @param st Status of items to open file descriptor for
 * @param flags Open flags
 * @return open file descriptor
 * @return -1 on error
 * @see open
 * @see open_items in the case *all* project items need to be opened/visible
 */
static_fn int open_items_status(enum status st, int flags) {
    switch (st) {
    case BACKLOG:
        return open(backlog_path, flags);
    case TODO:
        return open(todo_path, flags);
    case IN_PROG:
        return open(ip_path, flags);
    case DONE:
        return open(done_path, flags);
    default:
#ifdef DEBUG
        log_err("Unknown item state being requested for read");
#endif
        break;
    }
    return -1;
}

/*
 * @brief Get the user's home directory
 */
static_fn char *get_home_directory() {
    struct passwd *pw = getpwuid(getuid());
    return pw ? pw->pw_dir : NULL;
}

/*
 * @brief Check if a directory exists and is accessible
 */
static_fn int is_accessible_directory(const char *path) {
    if (access(path, F_OK | R_OK | W_OK) != 0) {
        return 0;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }

    return S_ISDIR(st.st_mode);
}

/*
 * @brief Move up one directory level in the path
 */
static_fn int move_up_directory(char *path) {
    char *last_slash = strrchr(path, '/');

    if (last_slash == NULL || last_slash == path) {
        return -1;
    }

    *last_slash = '\0';
    return 0;
}

/*
 * @brief Build relative path with appropriate number of "../" prefixes
 * @param dest
 * @param levels_up
 * @param target_dir
 */
static_fn void build_relative_path(char *dest, int levels_up,
                                   const char *target_dir) {
    dest[0] = '\0';

    if (levels_up == 0) {
        strcpy(dest, target_dir);
        return;
    }

    for (int i = 0; i < levels_up; i++) {
        if (i > 0)
            strcat(dest, "/");
        strcat(dest, "..");
    }
    strcat(dest, "/");
    strcat(dest, target_dir);
}

/*
 * @brief Search for target directory starting from current path, moving up
 * until home_dir
 */
static_fn int find_target_directory(const char *start_path,
                                    const char *home_dir,
                                    const char *target_dir, int *levels_up) {
    char search_path[MAX_PATH];
    char test_path[MAX_PATH + sizeof(CONF_PROJ_DIR)];

    strcpy(search_path, start_path);
    *levels_up = 0;

    while (*levels_up < MAX_PATH_LVLS) {
        /* Check if we've reached the home directory (exclusive) */
        if (strcmp(search_path, home_dir) == 0) {
            return -1;
        }

        /* Construct path to test */
        snprintf(test_path, sizeof(test_path), "%s/%s", search_path,
                 target_dir);

        /* Test if directory exists and is accessible */
        if (is_accessible_directory(test_path)) {
            return 0; /* Found it! */
        }

        /* Move up one directory level */
        if (move_up_directory(search_path) != 0) {
            return -1; /* Can't go up further */
        }

        (*levels_up)++;
    }

    return -1; /* Not found or too many levels */
}

int dir_find_project(char *dir) {
    assert(dir);

    char cwd[MAX_PATH];
    char *home_dir;
    int levels_up;

    /* Get current working directory */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return -1;
    }

    /* Get home directory */
    home_dir = get_home_directory();
    if (home_dir == NULL) {
        return -1;
    }

    /* Search for the target directory */

    if (find_target_directory(cwd, home_dir, CONF_PROJ_DIR, &levels_up) == 0) {
        /* Project has been found */
        /* Build the relative path */
        build_relative_path(dir, levels_up, CONF_PROJ_DIR);

        /* Store in static context for future dir_* calls */
        strcpy(proj_path, dir);

        return 0;
    }

    /* No project found */
    dir[0] = '\0';
    return -1;
}

void dir_construct_path(const char *const path, const char *base, char *buf,
                        const size_t max) {
    if (!path || !base) {
        buf = NULL;
        return;
    }
    size_t path_len = strlen(path);
    size_t base_len = strlen(base);

#ifdef DEBUG
    if (path_len + base_len >= max)
        log_err("Could not copy all data into buffer when constructing path");
#endif

    strncpy(buf, path, max);
    if (buf[strlen(buf) - 1] != '/' && path_len < max - 1) {
        strcat(buf, "/");
    }
    strncat(buf, base, max - strlen(buf));
}

int dir_init(const char *path) {
    /* No absolute paths permitted */
    assert(*path != '/');
    assert(*path != '~');

    /* Parent project directory */
    int ret = mkdir(path, CONF_DIR_PERMS);
    if (ret < 0)
        return ret;

    /* Set global variables */
    setup_path_names(path);

    /* Items */
    ret = create_items();

    int file_creation = create_file(next_id_path);
    dir_next_id(); /* Initialise ID */

    file_creation += create_file(listed_codes_path);
    file_creation += create_file(item_dependencies);

    /* Check file creation */
    if (file_creation != 0) {
#ifdef DEBUG
        log_err("Issue initalising project");
#endif
        ret = -1;
    }

    return ret;
}

/**
 * @brief Read an item entry and return all readable data in a freshly
 * allocated item.
 * @param entry Single entry representing item
 * @return Pointer to new item
 * @return NULL in case of error
 * @note Any data that cannot be ascertained from the entry will correspond
 * @see free
 */
item *entry_to_item(const char entry[DIR_ITEM_ENTRY_LEN + 1]) {
    item *item = item_init();
    size_t pos_in_entry = 0;
    /* Each step *may* be abstracted in the future for clarity */

    /*
     * Parse item ID:
     * Delimiter is guaranteed not to be a valid hex digit
     */
    const char *id = &entry[pos_in_entry];
    item->item_id = (sitem_id)strtoll(id, NULL, 16);
    pos_in_entry += HEX_LEN(sitem_id) + _DIR_ITEM_FIELD_DELIM_LEN;

    /*
     * Parse item code
     */
    const char *code = &entry[pos_in_entry];
    memcpy(item->item_code, code, ITEM_CODE_LEN);
    pos_in_entry += ITEM_CODE_LEN + _DIR_ITEM_FIELD_DELIM_LEN;

    /*
     * Parse item name
     */
    const char *name = &entry[pos_in_entry];
    /* pos_in_entry does not change; name is guaranteed to be the last field */

    int name_len = ITEM_NAME_MAX;

    /* Find true length of name, without filling spaces */
    for (int i = ITEM_NAME_MAX - 1; i > 0; i--) {
        if (name[i] != ' ') { /* Filler character is ' ' will not be modified */
            name_len = i + 2;
            break;
        }
    }

    /* See item_set_name_deep for null termination expectations */
    item_set_name_deep(item, name, name_len);

    return item;
}

/**
 * @brief Read item at an offset from fd as a new item struct
 * @param entry_off Offset of entry
 * @param fd File descriptor of file of item entries in regular format
 * @return item with data in entries file, heap allocated
 * @note entry_off must be the offset of the *first* byte of the item entry
 */
static_fn item *fd_read_item_at(int fd, off_t entry_off) {
    assert(fcntl(fd, F_GETFD) != -1); /* File descriptor is valid */
    assert(entry_off >= 0);

    char entry[DIR_ITEM_ENTRY_LEN + 1];
    entry[DIR_ITEM_ENTRY_LEN] = '\0';

    if (pread(fd, entry, DIR_ITEM_ENTRY_LEN, entry_off) < 0) {
        return NULL;
    }

    item *it = entry_to_item(entry);

    return it;
}

/**
 * @brief Get the total number of entries found in the file descriptor
 * @param fd File descriptor of entries (of any data)
 * @return Number of entries expected in fd
 * @return -1 on error
 * @note File contents are not read and thus, no entries can be verified
 */
static_fn int fd_total_items(const int fd, int entry_len) {
    assert(fcntl(fd, F_GETFD) != -1); /* File descriptor is valid */

    /* Use stat */
    struct stat sb;
    if (fstat(fd, &sb) < 0)
        return -1;

    return sb.st_size / entry_len;
}

int dir_total_items() {
    setup_path_names(NULL);

    /* Open and check item file descriptors */
    int item_fds[_DIR_ITEM_NUM_FILES];

    open_items(O_RDONLY, item_fds);

    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        if (item_fds[i] < 0) {
#ifdef DEBUG
            log_err("Could not open item file for reading");
#endif
            return -1;
        }
    }

    int num_items = 0;

    /* Read entries by examining file sizes of all item files */
    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        num_items += fd_total_items(item_fds[i], DIR_ITEM_ENTRY_LEN);
    }

    close_items(item_fds);

    return num_items;
}

/**
 * @brief Search for an entry with matching raw field data at a given position
 * in the entry given an open, readable file descriptor
 * @param fd Open file descriptor with the ability to read
 * @param entry_len Length of each entry in the file
 * @param pos_in_entry Position in entry (this effectively provides the field
 * being compared)
 * @param data Pointer to data expected in entry (does not have to be
 * null-terminated)
 * @param delim The delimiter to use to compare data to
 * @return Offset of matching entry in file fd
 * @return -1 if no matching entry exists
 * @note Returns the *first instance* where the field data matches the data
 * provided, be cautious (in fact, completely avoid) for fields which are not
 * necessarily unique.
 * @see dir.h For field positions in different entry formats
 * @see fd_search_for_entry_id For more efficient ID-based *item* searching
 */
static_fn off_t fd_find_entry_with_data(int fd, size_t entry_len,
                                        off_t pos_in_entry, const char *data,
                                        const char *delim) {
    assert(fcntl(fd, F_GETFD) != -1);
    const int flags = fcntl(fd, F_GETFL) & O_ACCMODE;
    assert(flags == O_RDWR || flags == O_RDONLY);
    assert(data);
    assert(pos_in_entry >= 0);

    int total_entries = fd_total_items(fd, entry_len);

    char curr_entry[entry_len + 1];
    int entry_found = 1;

    /* Read linearly */
    for (int i = 0; i < total_entries; i++) {
        entry_found = 1;
        pread(fd, curr_entry, sizeof(curr_entry) - 1, i * entry_len);
        /* Compare field data until next delimiter */
        char *start_cmp = curr_entry + pos_in_entry;
        char *end_cmp = strstr(start_cmp, delim);
        entry_found = strncmp(start_cmp, data, (end_cmp - start_cmp)) == 0;
        if (entry_found) { /* Matched field data */
            return i * entry_len;
        }
    }
    return -1;
}

/**
 * @brief Increment the next available ID in the NEXT_ID file
 * @param fd_next_id File descriptor open with read and write permissions for
 * next ID file.
 * @return Current ID (the one replaced)
 * @return -1 on error
 */
static_fn sitem_id increment_next_id(int fd_next_id) {
    assert((fcntl(fd_next_id, F_GETFL) & O_ACCMODE) == O_RDWR);

    char curr_id_hex_str[HEX_LEN(sitem_id) + 1];
    char next_id_hex_str[HEX_LEN(sitem_id) + 1];

    curr_id_hex_str[HEX_LEN(sitem_id)] = '\0'; /* Null terminate */
    next_id_hex_str[HEX_LEN(sitem_id)] = '\0';

    if (read(fd_next_id, curr_id_hex_str, HEX_LEN(sitem_id)) < 0) {
        close(fd_next_id);
        return -1;
    }

    sitem_id curr_id = (sitem_id)strtoll(curr_id_hex_str, NULL, 16);

    snprintf(next_id_hex_str, sizeof(next_id_hex_str), "%0*X",
             (int)HEX_LEN(sitem_id), curr_id + 1);

    if (pwrite(fd_next_id, next_id_hex_str, 2 * sizeof(sitem_id), 0) < 0) {
        close(fd_next_id);
        return -1;
    }

    ftruncate(fd_next_id, HEX_LEN(sitem_id));

    return curr_id;
}

sitem_id dir_next_id() {
    setup_path_names(NULL);

    const int fd_id = open(next_id_path, O_RDWR);

    struct stat sb;
    if (fstat(fd_id, &sb) < 0) {
        close(fd_id);
        return -2;
    }

    char start_index[HEX_LEN(sitem_id) + 1];
    memset(start_index, '0', sizeof(start_index) - 1);
    start_index[sizeof(start_index) - 1] = '\0'; /* Null terminate */

    if (sb.st_size == 0) {
        int ret = -1;
        /* Initialise project with start_index */
        if (write(fd_id, start_index, sizeof(start_index) - 1) < 0)
            ret = -2; /* Error case */
        close(fd_id);
        return ret;
    }

    /* Increment next available ID */
    sitem_id curr_id = increment_next_id(fd_id);

    close(fd_id);
    return curr_id;
}

int dir_contains_item_with_id(sitem_id id) {
    setup_path_names(NULL);
    if (id < 0)
        return 0;

    int item_fds[_DIR_ITEM_NUM_FILES];
    open_items(O_RDONLY, item_fds);
    char id_str[HEX_LEN(sitem_id) + 1];
    snprintf(id_str, sizeof(id_str), "%0*X", (int)HEX_LEN(sitem_id), id);
    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        if (fd_find_entry_with_data(item_fds[i], DIR_ITEM_ENTRY_LEN, 0, id_str,
                                    _DIR_ITEM_FIELD_DELIM) >= 0) {
            return 1;
        }
    }
    return 0;
}

item **dir_read_items_status(enum status st) {
    setup_path_names(NULL);

    const int rd_flags = O_RDONLY;
    int fd = open_items_status(st, rd_flags);
    if (fd == -1)
        return NULL;

    int total_items = fd_total_items(fd, DIR_ITEM_ENTRY_LEN);
    item **items = (item **)malloc(sizeof(item *) * (total_items + 1));
    if (!items)
        return NULL;

    char item_entry[DIR_ITEM_ENTRY_LEN + 1];

    for (int i = 0; i < total_items; i++) {
        if (pread(fd, item_entry, sizeof(item_entry), i * DIR_ITEM_ENTRY_LEN) <
            0) {
#ifdef DEBUG
            if (errno == EINVAL)
                log_err("Attempting to read beyond items file");
#endif
            break; /* Could not read for some reason -- this is ignored */
        }
        item_entry[DIR_ITEM_ENTRY_LEN] = '\0'; /* Not done by pread */
        items[i] = entry_to_item(item_entry);  /* Parse entry data */
        items[i]->item_st = st;                /* Set status */
    }

    /* NULL terminate */
    items[total_items] = NULL;

    return items;
}

item **dir_read_all_items() {
    setup_path_names(NULL);

    int total_items = dir_total_items();
    /* Array of items */
    item **items = item_array_init_empty(total_items);

    if (!items) {
        puts("Could not read any items");
#ifdef DEBUG
        log_err("dir_read_all_items: malloc call failed, check item entries");
#endif
        return NULL;
    }

    unsigned int items_read = 0;

    /* Iterate for all status types */
    for (int i = 0; i < ITEM_STATUS_COUNT; i++) {
        item **items_of_same_st = dir_read_items_status((enum status)i);

        /* Copy NULL-terminated pointer array to total collection */
        for (size_t j = 0; (items[items_read] = items_of_same_st[j]) != NULL;
             j++) {
            items[items_read]->item_st = (enum status)i; /* Set status */
            items_read++;
        }

        free(items_of_same_st);
    }

    return items;
}

/**
 * @brief Write an item entry to buf
 * @param itp Pointer to item to parse data of
 * @param buf Buffer to place data
 * @note Resulting data in buf is null-terminated and guaranteed to at the last
 * position
 */
static_fn void make_item_entry(const item *const itp,
                               char buf[DIR_ITEM_ENTRY_LEN + 1]) {
    /* Field delimiter */
    const char delim[_DIR_ITEM_FIELD_DELIM_LEN + 1] = _DIR_ITEM_FIELD_DELIM;
    /* Item terminator */
    const char term[_DIR_ITEM_DELIM_LEN + 1] = _DIR_ITEM_DELIM;

    /* Bytes printed to buf */
    int b = 0;

    /* Parse item data */
    b = snprintf(buf, DIR_ITEM_ENTRY_LEN + 1, "%0*X%s%.*s%s%-*s%s",
                 /* Width and value of ID */
                 (int)HEX_LEN(sitem_id), itp->item_id, delim, ITEM_CODE_LEN,
                 itp->item_code, delim, ITEM_NAME_MAX,
                 itp->item_name, /* Name */
                 term);

    /* Handle errors */
    if ((size_t)b < DIR_ITEM_ENTRY_LEN) {
        printf("Unable to save item, names must be less than %d characters\n",
               ITEM_NAME_MAX);
#ifdef DEBUG
        log_err("make_item_entry could not parse item data correctly");
#endif
    }
}

/**
 * @brief Helper for fd_search_for_entry_id -- does not assert invariants and is
 * therefore potentially dangerous to use in other contexts
 * @return >= 0 offset if item found
 * @return < 0 offset if item is not found - position of where it *should* be
 * @note Does not deal with empty file case
 */
static_fn off_t _fd_bin_search_entry_id(const int fd, const sitem_id target,
                                        off_t start, off_t end) {
    off_t middle = (start + end) / 2 -
                   /* Align to item entry offset */
                   (((start + end) / 2) % DIR_ITEM_ENTRY_LEN);

    /* This may cause some overhead in *very* large projects; future concern */
    item *start_item = fd_read_item_at(fd, start);
    item *mid_item = fd_read_item_at(fd, middle);
    item *end_item = fd_read_item_at(fd, end);

    sitem_id sid = start_item->item_id;
    sitem_id mid = mid_item->item_id;
    sitem_id eid = end_item->item_id;

    item_free(start_item);
    item_free(mid_item);
    item_free(end_item);

    /* Found cases */
    if (sid == target)
        return start;
    if (eid == target)
        return end;
    if (start == end && sid == target)
        return start;

    /* 'Out of bounds' cases */
    if (sid > target) {
        if (start == 0)
            return OFF_T_MIN; /* "-0" case */
        return -start;
    }
    if (eid < target) {
        return -(end + DIR_ITEM_ENTRY_LEN);
    }
    if (end - start == DIR_ITEM_ENTRY_LEN) {
        /* Item should be between two others */
        return -end;
    }

    if (mid < target)
        start = middle;
    else
        end = middle;

    return _fd_bin_search_entry_id(fd, target, start, end);
}

/**
 * @brief Search for an entry in a file, regardless of whether it exists or not
 * @param fd Open file descriptor
 * @param target_id Target item ID to find
 * @return >= 0: offset of location of item with target_id in fd;
 * @return < 0: offset of location that an item with target_id *would be*,
 * since "-0" is obviously indistinguishable from 0, OFF_T_MIN represents a
 * 'would-be' insertion offset of 0; this is always returned for an empty file.
 * @return -1 on error (guaranteed not to align with a valid 'negative offset'
 */
static_fn off_t fd_search_for_entry_id(const int fd, const sitem_id target_id) {
    /* Conduct binary search on open fd */
    assert(fcntl(fd, F_GETFD) != -1);
    int flags = fcntl(fd, F_GETFL) & O_ACCMODE;

    if (flags != O_RDWR && flags != O_RDONLY) {
#ifdef DEBUG
        log_err("Incorrect fd flag provided");
#endif
        return -1;
    }
    if (target_id < 0)
        return -1;

    off_t last_off =
        (fd_total_items(fd, DIR_ITEM_ENTRY_LEN) - 1) * DIR_ITEM_ENTRY_LEN;

    /* Empty file */
    if (last_off < 0)
        return OFF_T_MIN;

    /* Commence search on whole file */
    return _fd_bin_search_entry_id(fd, target_id, 0, last_off);
}

/**
 * @brief Find item with matching field data in project
 * @param pos_in_entry Position in entry expected
 * @param data Raw data expected in entry
 * @return Pointer to heap allocated item with corresponding data
 * @return NULL if item cannot be found
 */
static_fn item *find_item_matching_field(const off_t pos_in_entry,
                                         const char *data) {
    int item_fds[_DIR_ITEM_NUM_FILES];

    item *itp = NULL;

    open_items(O_RDONLY, item_fds);

    for (int i = 0; i < ITEM_STATUS_COUNT; i++) {
        if (item_fds[i] < 0) {
#ifdef DEBUG
            log_err("Could not open item files for reading and writing");
#endif
            return itp;
        }

        off_t item_offset = fd_find_entry_with_data(
            item_fds[i], DIR_ITEM_ENTRY_LEN, pos_in_entry,
            /* Will not work with *last* field */
            data, _DIR_ITEM_FIELD_DELIM);
        if (item_offset >= 0) {
            char correct_entry[DIR_ITEM_ENTRY_LEN + 1];
            if (pread(item_fds[i], correct_entry, DIR_ITEM_ENTRY_LEN,
                      item_offset) < 0)
                return NULL;
            itp = entry_to_item(correct_entry);
        }
        if (itp)
            break;
    }

    close_items(item_fds);

    return itp;
}

/**
 * @brief Swap item entries in a file of item entries opened with fd
 * @param fd File descriptor of open file of item entries
 * @param off_a First given offset of entry to swap
 * @param off_b Second given offset of entry to swap
 * @return 0 on success
 * @return -1 if offsets are incorrect or in case of some other error
 * @note Handles case where off_a == off_b
 */
static_fn int fd_swap_item_entries_at(const int fd, const off_t off_a,
                                      const off_t off_b) {
    assert(fcntl(fd, F_GETFD) != -1); /* File descriptor is valid */

    /* Check proper flags */
    if ((fcntl(fd, F_GETFL) & O_ACCMODE) != O_RDWR) {
#ifdef DEBUG
        log_err("Incorrect fd flag provided");
#endif
        return -1;
    }

    /* Check offsets are valid */
    off_t total_item_size =
        fd_total_items(fd, DIR_ITEM_ENTRY_LEN) * DIR_ITEM_ENTRY_LEN;
    /*
     * NOTE: NULL arg could produce undefined behaviour without previous call
     * to setup_path_names -- will be amended in future changes
     */

    if (off_a < 0 || off_a >= total_item_size || off_b < 0 ||
        off_b >= total_item_size) {
        return -1;
    }

    if (off_a == off_b)
        return 0;

    char temp_entry_a[DIR_ITEM_ENTRY_LEN];
    char temp_entry_b[DIR_ITEM_ENTRY_LEN];

    /* Read item entries to buffer */
    if (pread(fd, temp_entry_a, sizeof(temp_entry_a), off_a) < 0 ||
        pread(fd, temp_entry_b, sizeof(temp_entry_b), off_b) < 0) {
        return -1;
    }

    /* Write back in swapped positions */
    if (pwrite(fd, temp_entry_b, sizeof(temp_entry_b), off_a) < 0 ||
        pwrite(fd, temp_entry_a, sizeof(temp_entry_a), off_b) < 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Append write the item entry of the item pointed to by itp to the
 * appropriate items file
 * @param itp Pointer to item to write data of
 * @param entry Formatted entry of item to write with terminating NULL
 * character
 * @return 0 on success
 * @return -1 on error
 * @note No 'correctness' checks occur to validate that the entry indeed
 * represents the item pointed to by itp, this is assumed to be the case
 */
static_fn int append_item_entry(const item *itp,
                                const char entry[DIR_ITEM_ENTRY_LEN + 1]) {
    int open_flags = O_RDWR;
    int fd = open_items_status(itp->item_st, open_flags);
    if (fd == -1)
        return -1;

    /* Ensure that the new item is placed in order */
    off_t new_item_pos = fd_search_for_entry_id(fd, itp->item_id);
    assert(new_item_pos < 0);
    /* Make non-negative */
    new_item_pos = (new_item_pos == OFF_T_MIN) ? 0 : -new_item_pos;

    char replaced_entry[DIR_ITEM_ENTRY_LEN];

    /* Read replaced entry */
    if (pread(fd, replaced_entry, sizeof(replaced_entry), new_item_pos) < 0) {
        return -1;
    }
    off_t eof_pos = lseek(fd, 0, SEEK_END);
    /* Append replaced entry */
    if (pwrite(fd, replaced_entry, sizeof(replaced_entry), eof_pos) < 0) {
        return -1;
    }
    /* Write new entry in old position */
    if (pwrite(fd, entry, sizeof(replaced_entry), new_item_pos) < 0) {
        return -1;
    }

    /* Bubble old entry back up */
    for (off_t i =
             (fd_total_items(fd, DIR_ITEM_ENTRY_LEN) - 1) * DIR_ITEM_ENTRY_LEN;
         i > new_item_pos + (off_t)DIR_ITEM_ENTRY_LEN;
         i -= DIR_ITEM_ENTRY_LEN) {
        fd_swap_item_entries_at(fd, i, i - DIR_ITEM_ENTRY_LEN);
    }
    syncfs(fd);
    return 0;
}

int dir_append_item(const item *it) {
    assert(it != NULL);
    setup_path_names(NULL);

    /* Additional + 1 allocated for NULL byte */
    char item_entry[DIR_ITEM_ENTRY_LEN + 1] = {'\0'};

    make_item_entry(it, item_entry);

    /* Write item data to items file */
    if (append_item_entry(it, item_entry) < 0)
        return -1;

    return 0;
}

/**
 * @brief Remove entry at given offset
 * @param fd File descriptor of file of entries (of any data) in regular format
 * @param entry_off Offset of entry
 * @param entry_len Length of a single entry in the file opened
 * @return 0 on success
 * @return -1 on error or if file if opened by fd is empty
 * @note entry_off must be the offset of the *first* character of the item
 * entry
 * @note Preserve ordering of entries (besides removed one, obviously)
 */
static_fn int fd_remove_entry_at(const int fd, const off_t entry_off,
                                 int entry_len) {
    assert(fcntl(fd, F_GETFD) != -1); /* File descriptor is valid */
    assert(entry_off >= 0);

    off_t last_off = (fd_total_items(fd, entry_len) - 1) * entry_len;
    if (entry_off > last_off) {
#ifdef DEBUG
        log_err("Entry offset provided too large given item file size");
#endif
        return -1;
    }

    /* Bubble entry to end of file */
    for (off_t i_off = entry_off; i_off < last_off; i_off += entry_len) {
        fd_swap_item_entries_at(fd, i_off, i_off + entry_len);
    }

    /* Truncate file */
    if (ftruncate(fd, last_off) < 0) {
#ifdef DEBUG
        log_err("Item entry file could not be truncated when removing entry");
#endif
        return -1;
    }

    return 0;
}

int dir_change_item_status_id(const sitem_id id, const enum status new_status) {
    assert(new_status < ITEM_STATUS_COUNT);           /* Validate status */
    assert(_DIR_ITEM_NUM_FILES == ITEM_STATUS_COUNT); /* Expected structure */

    setup_path_names(NULL);

    int item_fds[_DIR_ITEM_NUM_FILES];

    item *itp = NULL;

    /* Find item in project */
    open_items(O_RDWR, item_fds);

    for (int i = 0; i < ITEM_STATUS_COUNT; i++) {
        if (item_fds[i] < 0) {
#ifdef DEBUG
            log_err("Could not open item files for reading and writing");
#endif
            return -1;
        }
        off_t item_off = fd_search_for_entry_id(item_fds[i], id);

        if (item_off >= 0) {
            /* Status is already correct - exit from function */
            if ((enum status)i == new_status)
                break;

            int fd = item_fds[i];

            /* Remove item from current location */
            itp = fd_read_item_at(fd, item_off);

            if (!itp) { /* Could not read item */
                close_items(item_fds);
                return -1;
            }

            fd_remove_entry_at(fd, item_off, DIR_ITEM_ENTRY_LEN);
        }
#ifdef DEBUG
        else if (item_off == -1) {
            log_err("Issue when searching for item");
        }
#endif
    }

    /* Item does not exist */
    if (!itp)
        return -1;

    /* Update status */
    itp->item_st = new_status;

    /* Add to new location */
    dir_append_item(itp);

    close_items(item_fds);

    item_free(itp);

    return 0;
}

void dir_write_item_codes(item *const *items, const int *prefix_lengths) {
    assert(items != NULL);
    assert(prefix_lengths != NULL);

    setup_path_names(NULL);

    int fd_item_codes = open(listed_codes_path, O_WRONLY | O_TRUNC);
    /* Item codes will be structured according to the following: */
    char curr_code_entry[_DIR_CODE_ENTRY_LEN + 1];

    for (int i = 0; items[i]; i++) {
        int pref_len = prefix_lengths[i];
        assert(prefix_lengths[i] > 0 && prefix_lengths[i] <= ITEM_CODE_CHARS);

        int b = snprintf(curr_code_entry, sizeof(curr_code_entry),
                         "%0*X%s%-*.*s%*s%s", (int)HEX_LEN(sitem_id),
                         items[i]->item_id, _DIR_ITEM_FIELD_DELIM, pref_len,
                         pref_len, items[i]->item_code,
                         ITEM_CODE_LEN - pref_len, "", _DIR_ITEM_DELIM);

        if ((size_t)b < sizeof(curr_code_entry) - 1) {
#ifdef DEBUG
            log_err("A code entry could not be created for listed entries");
#endif
            break;
        }

        if (write(fd_item_codes, curr_code_entry, sizeof(curr_code_entry) - 1) <
            0)
            break;
    }

    close(fd_item_codes);
}

/**
 * @brief Check if the prefix matches the expected prefix
 * @param prefix Prefix of code
 * @param expected Expected prefix
 * @note Expected prefix (expected) must be null terminated
 */
static_fn int code_prefix_matches(const char *prefix, const char *expected) {
    assert(prefix);
    assert(expected);

    /* Character to read until in prefix */
    int matches = 1;

    for (int i = 0; i < ITEM_CODE_LEN && expected[i] != '\0'; i++) {
        if (prefix[i] != expected[i]) {
            matches = 0;
            break;
        }
    }

    return matches;
}

sitem_id dir_get_id_from_prefix(const char *code_prefix) {
    if (!code_prefix)
        return -1;

    setup_path_names(NULL);

    struct stat sb;
    if (stat(listed_codes_path, &sb) < 0)
        return -1;
    if (sb.st_size == 0)
        return -1;

    sitem_id found_id = -1;

    /* Search last listed code prefixes */
    int fd_listed_prefixes = open(listed_codes_path, O_RDONLY);
    char curr_code_entry[_DIR_CODE_ENTRY_LEN];
    int num_items_listed = sb.st_size / sizeof(curr_code_entry);

    for (int i = 0; i < num_items_listed; i++) {
        pread(fd_listed_prefixes, curr_code_entry, sizeof(curr_code_entry),
              i * sizeof(curr_code_entry));

        if (code_prefix_matches(
                &curr_code_entry[HEX_LEN(sitem_id) + _DIR_ITEM_FIELD_DELIM_LEN],
                code_prefix)) {
            found_id = strtoll(curr_code_entry, NULL, 16);
            break;
        }
    }
    close(fd_listed_prefixes);

    return found_id;
}

item *dir_get_item_with_code(const char *full_code) {
    assert(full_code);

    if (item_is_valid_code(full_code) < 0)
        return NULL;

    setup_path_names(NULL);

    item *itp = find_item_matching_field(
        HEX_LEN(sitem_id) + _DIR_ITEM_FIELD_DELIM_LEN, full_code);

    return itp;
}

/**
 * @brief Read buffered dependency string entry into a dependency struct
 * @param dep Pointer to dependency struct to write to
 * @param buffer Buffer representing formatted dependency entry
 */
static_fn void read_dependency(struct dependency *dep, const char *buf) {
    assert(buf);
    assert(dep);

    size_t buffer_index = 0;

    /* Parse *to* item ID - Delimiter is not in any part a valid hex digit */
    dep->to = (sitem_id)strtoll(&buf[buffer_index], NULL, 16);
    buffer_index += HEX_LEN(sitem_id) + _DIR_ITEM_FIELD_DELIM_LEN;

    /* Parse *from* ID */
    dep->from = (sitem_id)strtoll(&buf[buffer_index], NULL, 16);
    buffer_index += HEX_LEN(sitem_id) + _DIR_ITEM_FIELD_DELIM_LEN;

    /* Parse is_ghost */
    dep->is_ghost = (int)strtoll(&buf[buffer_index], NULL, 2);
}

/**
 * @brief Read dependency to string buffer
 * @param dep Dependency to make entry for
 * @param buf Buffer to write entry to
 */
static_fn void dependency_to_entry(const struct dependency *const dep,
                                   char *buf) {
    assert(buf);
    assert(dep);

    /* Field delimiter */
    const char delim[_DIR_ITEM_FIELD_DELIM_LEN + 1] = _DIR_ITEM_FIELD_DELIM;
    /* Item terminator */
    const char term[_DIR_ITEM_DELIM_LEN + 1] = _DIR_ITEM_DELIM;

    int b =
        snprintf(buf, _DIR_DEPENDENCY_ENTRY_LEN + 1, "%0*X%s%0*X%s%d%s",
                 (int)HEX_LEN(sitem_id), dep->to, delim, (int)HEX_LEN(sitem_id),
                 dep->from, delim, dep->is_ghost > 0, term);

#ifdef DEBUG
    if ((size_t)b < _DIR_DEPENDENCY_ENTRY_LEN)
        log_err("Could not write dependency item as a string entry");
#endif
}

struct dependency_list *dir_get_all_dependencies() {
    setup_path_names(NULL);

    int fd = open(item_dependencies, O_RDONLY);
    const int total_dependencies =
        fd_total_items(fd, _DIR_DEPENDENCY_ENTRY_LEN);
    if (total_dependencies < 0)
        return NULL;
    char dependency_entry[_DIR_DEPENDENCY_ENTRY_LEN] = {0};

    struct dependency_list *list =
        graph_init_dependency_list(total_dependencies);

    struct dependency *new_dependency = NULL;

    for (int i = 0; i < total_dependencies; i++) {
        new_dependency = graph_new_dependency(-1, -1, 0);
        if (pread(fd, dependency_entry, sizeof(dependency_entry),
                  i * _DIR_DEPENDENCY_ENTRY_LEN) == -1) {
#ifdef DEBUG
            log_err("Unable to read item dependencies");
#endif
            close(fd);
            return NULL;
        }
        read_dependency(new_dependency, dependency_entry);
        graph_new_dependency_to_list(list, &new_dependency);
    }
    close(fd);
    return list;
}

void dir_add_dependency_list(const struct dependency_list *const list) {
    assert(list);
    for (unsigned int i = 0; i < list->count; i++) {
        dir_add_dependency(list->dependencies[i]);
    }
}

void dir_add_dependency(const struct dependency *const dep) {
    assert(dep);
    if (dep->from < 0 || dep->to < 0) {
#ifdef DEBUG
        log_err("The from or to IDs provided were invalid");
#endif
        return;
    }

    setup_path_names(NULL);

    int fd = open(item_dependencies, O_WRONLY | O_APPEND);
    char dependency_entry[_DIR_DEPENDENCY_ENTRY_LEN + 1];
    dependency_to_entry(dep, dependency_entry);

    if (write(fd, dependency_entry, _DIR_DEPENDENCY_ENTRY_LEN) == -1) {
#ifdef DEBUG
        log_err("Unable to write dependency");
#endif
    }
    close(fd);
}

int dir_rm_dependency(const struct dependency *const dep) {
    assert(dep);
    if (dep->from < 0 || dep->to < 0) {
#ifdef DEBUG
        log_err("The from or to IDs provided were invalid");
#endif
        return -1;
    }
    int fd = open(item_dependencies, O_WRONLY);

    char entry[_DIR_DEPENDENCY_ENTRY_LEN];
    dependency_to_entry(dep, entry);
    off_t entry_pos = fd_find_entry_with_data(fd, _DIR_DEPENDENCY_ENTRY_LEN, 0,
                                              entry, _DIR_ITEM_DELIM);
    if (entry_pos < 0) {
        return -1;
    }

    if (fd_remove_entry_at(fd, entry_pos, _DIR_ITEM_FIELD_DELIM_LEN) < 0) {
#ifdef DEBUG
        log_err("Could not remove entry at given location");
#endif
        return -2;
    }
    return 0;
}
