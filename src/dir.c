#include "dir.h"
#include "config.h"
#include "tojo.h"
#include "ds/item.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Project directory */
static char proj_path[_MAX_PATH] =  { '\0' };

/* Item storage */
static char items_path[_MAX_PATH] = { '\0' };
static char todo_path[_MAX_PATH] =  { '\0' };
static char ip_path[_MAX_PATH] =    { '\0' };
static char done_path[_MAX_PATH] =  { '\0' };

static char next_id_path[_MAX_PATH] = { '\0' }; /* Next available item ID */

/**
 * @brief Set up global path variables to default values if not already done
 * so. Should be one of the first calls made by dir_* functions (excluding) for
 * project initialisation.
 * @param path Path to project directory may be NULL
 * @note Modifies global variables, should be used once in a dir API call
 * @note Calling with NULL assumes that project path is already known
 */
static void setup_path_names(const char * const path) {
    if (!*proj_path && path)
        strcpy(proj_path, path);

    /* Set up items path */
    if (!*items_path)
        dir_construct_path(proj_path, _DIR_ITEM_PATH_D, items_path, _MAX_PATH);

    /* Item file names */
    if (!*todo_path)
        dir_construct_path(items_path, _DIR_ITEM_TODO_F, todo_path, _MAX_PATH);
    if (!*ip_path)
        dir_construct_path(items_path, _DIR_ITEM_INPROG_F, ip_path, _MAX_PATH);
    if (!*done_path)
        dir_construct_path(items_path, _DIR_ITEM_DONE_F, done_path, _MAX_PATH);

    /* Next ID data */
    if (!*next_id_path)
        dir_construct_path(proj_path, _DIR_NEXT_ID_F, next_id_path, _MAX_PATH);
}

/**
 * @brief Create a file given by fname; used for project files.
 * @param fname Name of file to create
 * @return 0 for successful file creation
 * @return 1 in case of EEXIST
 * @return 2 in case of other error
 */
static int create_file(const char *const fname) {
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
static int create_items() {
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
    int file_creation =
        create_file(todo_path) + create_file(ip_path) + create_file(done_path);

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
static void open_items(const int flags, int item_fds[_DIR_ITEM_NUM_FILES]) {
    assert(strlen(items_path)
           >= sizeof(CONF_PROJ_DIR) + sizeof(_DIR_ITEM_PATH_D) - 1);

    if (!item_fds)
        return;

    item_fds[0] = open(todo_path, flags);
    item_fds[1] = open(ip_path, flags);
    item_fds[2] = open(done_path, flags);

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
static void close_items(const int item_fds[_DIR_ITEM_NUM_FILES]) {
    if (!item_fds)
        return;

    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        close(item_fds[i]);
    }
}

/*
 * @brief Get the user's home directory
 */
static char* get_home_directory(void) {
    struct passwd *pw = getpwuid(getuid());
    return pw ? pw->pw_dir : NULL;
}

/*
 * @brief Check if a directory exists and is accessible
 */
static int is_accessible_directory(const char *path) {
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
static int move_up_directory(char *path) {
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
static void build_relative_path(char *dest, int levels_up,
                                const char *target_dir) {
    dest[0] = '\0';
    
    if (levels_up == 0) {
        strcpy(dest, target_dir);
        return;
    }
    
    for (int i = 0; i < levels_up; i++) {
        if (i > 0) strcat(dest, "/");
        strcat(dest, "..");
    }
    strcat(dest, "/");
    strcat(dest, target_dir);
}

/*
 * @brief Search for target directory starting from current path, moving up
 * until home_dir
 */
static int find_target_directory(const char *start_path, const char *home_dir, 
                                 const char *target_dir, int *levels_up) {
    char search_path[_MAX_PATH];
    char test_path[_MAX_PATH + sizeof(CONF_PROJ_DIR)];
    
    strcpy(search_path, start_path);
    *levels_up = 0;
    
    while (*levels_up < _MAX_PATH_LVLS) {
        /* Check if we've reached the home directory (exclusive) */
        if (strcmp(search_path, home_dir) == 0) {
            return -1;
        }
        
        /* Construct path to test */
        snprintf(test_path, sizeof(test_path), "%s/%s", search_path,
                 target_dir);
        
        /* Test if directory exists and is accessible */
        if (is_accessible_directory(test_path)) {
            return 0;  /* Found it! */
        }
        
        /* Move up one directory level */
        if (move_up_directory(search_path) != 0) {
            return -1;  /* Can't go up further */
        }
        
        (*levels_up)++;
    }
    
    return -1;  /* Not found or too many levels */
}

int dir_find_project(char *dir) {
    assert(dir);
    
    char cwd[_MAX_PATH];
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

void dir_construct_path(const char * const path, const char *base, char *buf,
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
    if (ret < 0) return ret;

    /* Set global variables */
    setup_path_names(path);

    /* Items */
    ret = create_items();

    /* ID */
    int file_creation = create_file(next_id_path);
    dir_next_id();

    /* Check file creation */
    if (file_creation != 0) {
        ret = -1;
    }

    return ret;
}

/**
 * @brief Read item name from file opened by file descriptor fd which contains
 * item data
 * @return Pointer to allocated item
 * @return NULL if no matching item is found
 * @see item_init, item_free
 */
static item * fd_read_item(const char *name, const int fd) {
    assert(name); /* Name exists */
    assert(fd > 0); /* Valid file descriptor */
    /* Has read perms */
    assert((fcntl(fd, F_GETFL) & O_ACCMODE) != O_WRONLY);

    size_t name_len = strlen(name);

    /* Read item from file */
    /* Assumes that each line contains only the item name */
    char buf[ITEM_NAME_MAX];
    char *next_name;
    ssize_t bytes;

    /* Read lines */
    while ((bytes = read(fd, buf, sizeof(buf) - 1)) > 0) {
        if (bytes < 0)
            return NULL;

        buf[bytes] = '\0';

        /* Extract Names */
        next_name = strtok(buf, "\n");
        while (next_name != NULL) {
            if (strncmp(buf, name, name_len) == 0) {
                item *itp = item_init();
                item_set_name_deep(itp, name, ITEM_NAME_MAX);
                return itp;
            }
            next_name = strtok(NULL, "\n");
        }
    }

    return NULL;
}

item * dir_find_item(const char *name) {
    setup_path_names(NULL);

    /* Item file descriptors open for reading */
    int item_fds[_DIR_ITEM_NUM_FILES];

    open_items(O_RDONLY, item_fds);

    /* Simple opening check */
    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        if (item_fds[i] < 0) {
#ifdef DEBUG
            log_err("Could not open item file for reading");
#endif
            return NULL;
        }
    }

    item *itp = NULL;

    for (int i = 0; i < _DIR_ITEM_NUM_FILES && !itp; i++)
        itp = fd_read_item(name, item_fds[i]);

#ifdef DEBUG
    if (itp == NULL) {
        log_err("Item could not be allocated");
    }
#endif

    close_items(item_fds);

    return itp;
}
/**
 * @brief Get the total number of item entries found in the file descriptor
 * @param fd File descriptor of 
 * @return Number of item entries expected in fd
 * @return -1 on error
 * @note File contents are not read and thus, no entries can be verified
 */
static int fd_total_items(const int fd) {
    assert(fcntl(fd, F_GETFD) != -1); /* File descriptor is valid */
    
    /* Use stat */
    struct stat sb;
    if (fstat(fd, &sb) < 0)
        return -1;

    return sb.st_size / DIR_ITEM_ENTRY_LEN;
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
        num_items += fd_total_items(item_fds[i]);
    }

    close_items(item_fds);

    return num_items;
}

/**
 * @brief Increment the next available ID in the NEXT_ID file
 * @param fd_next_id File descriptor open with read and write permissions for
 * next ID file.
 * @return Current ID (the one replaced)
 * @return -1 on error
 */
static ssize_t increment_next_id(int fd_next_id, int id_hex_width) {
    assert((fcntl(fd_next_id, F_GETFL) & O_ACCMODE) == O_RDWR);


    char curr_id_hex_str[2 * sizeof(ssize_t) + 1];
    char next_id_hex_str[2 * sizeof(ssize_t) + 1];

    curr_id_hex_str[2 * sizeof(ssize_t)] = '\0'; /* Null terminate */
    next_id_hex_str[2 * sizeof(ssize_t)] = '\0';

    if (read(fd_next_id, curr_id_hex_str, 2 * sizeof(ssize_t)) < 0) {
        close(fd_next_id);
        return -1;
    }

    ssize_t curr_id = (ssize_t) strtoll(curr_id_hex_str, NULL, 16);

    snprintf(next_id_hex_str, sizeof(next_id_hex_str), "%0*zX",
             id_hex_width, curr_id + 1);

    if (pwrite(fd_next_id, next_id_hex_str, 2 * sizeof(ssize_t), 0) < 0) {
        close(fd_next_id);
        return -1;
    }

    ftruncate(fd_next_id, id_hex_width);

    return curr_id;
}

ssize_t dir_next_id() {
    setup_path_names(NULL);

    const int fd_id = open(next_id_path, O_RDWR);

    struct stat sb;
    if (fstat(fd_id, &sb) < 0) {
        close(fd_id);
        return -2;
    }

    const unsigned int long_hex_width = (unsigned int) sizeof(ssize_t) * 2;

    if (sb.st_size == 0) {
        /* Initialise project */
        for (unsigned int i = 0; i < long_hex_width; i++)
            write(fd_id, "0", 1);
        close(fd_id);
        return -1;
    }

    /* Increment next available ID */
    ssize_t curr_id = increment_next_id(fd_id, long_hex_width);

    close(fd_id);
    return curr_id;
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

    /*
     * Parse item ID
     * Delimiter is guaranteed not to be a valid hex digit
     */
    const char *id = &entry[0];
    item->item_id = (ssize_t) strtoll(id, NULL, 16);

    /* Parse item name */
    const char *name = &entry[sizeof(ssize_t) * 2 + _DIR_ITEM_FIELD_DELIM_LEN];

    int name_len = ITEM_NAME_MAX;

    /* Find true length of name */
    for (int i = ITEM_NAME_MAX - 1; i > 0; i--) {
        if (name[i] != ' ') { /* Filler character is ' ', may be modified */
            name_len = i + 2;
            break;
        }
    }

    /* See item_set_name_deep for null termination expectations */
    item_set_name_deep(item, name, name_len);

    return item;
}

item ** dir_read_items_status(enum status st) {
    setup_path_names(NULL);

    const int rd_flags = O_RDONLY;
    int fd;

    /* This is to be refactored from switch-case statement */
    switch (st) {
    case TODO:
        fd = open(todo_path, rd_flags);
        break;
    case IN_PROG:
        fd = open(ip_path, rd_flags);
        break;
    case DONE:
        fd = open(done_path, rd_flags);
        break;
#ifdef DEBUG
    default:
        log_err("Unknown item state being requested for read");
#endif
    }

    int total_items = fd_total_items(fd);
    item **items = (item **) malloc(sizeof(item *) * (total_items + 1));
    if (!items)
        return NULL;

    char item_entry[DIR_ITEM_ENTRY_LEN + 1];

    for (int i = 0; i < total_items; i++) {
        if (pread(fd, item_entry, sizeof(item_entry), i * DIR_ITEM_ENTRY_LEN)
            < 0) {
#ifdef DEBUG
            if (errno == EINVAL)
                log_err("Attempting to read beyond items file");
#endif
            break; /* Could not read for some reason -- this is ignored */
        }
        item_entry[DIR_ITEM_ENTRY_LEN] = '\0'; /* Not done by pread */
        items[i] = entry_to_item(item_entry); /* Parse entry data */
        items[i]->item_st = st; /* Set status */
    }

    /* NULL terminate */
    items[total_items] = NULL;

    return items;
}

item ** dir_read_all_items() {
    setup_path_names(NULL);

    int total_items = dir_total_items();
    /* Array of items */
    item **items = (item **) malloc(sizeof(item *)
                                    * (total_items + 1));

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
        item **items_of_same_st = dir_read_items_status((enum status) i);

        /* Copy NULL-terminated pointer array to total collection */
        for (size_t j = 0; (items[items_read] = items_of_same_st[j]) != NULL;
             j++) {
            items[items_read]->item_st = (enum status) i; /* Set status */
            items_read++;
        }

        free(items_of_same_st);
    }

    /* Set last item pointer to NULL */
    items[total_items] = NULL;

    return items;
}

/**
 * @brief Write an item entry to buf
 * @param itp Pointer to item to parse data of
 * @param buf Buffer to place data
 * @note Resulting data in buf is null-terminated and guaranteed to at the last
 * position
 */
static void make_item_entry(const item *const itp,
                           char buf[DIR_ITEM_ENTRY_LEN + 1]) {
    /* Field delimiter */
    const char delim[_DIR_ITEM_FIELD_DELIM_LEN + 1] = _DIR_ITEM_FIELD_DELIM;
     /* Item terminator */
    const char term[_DIR_ITEM_DELIM_LEN + 1] = _DIR_ITEM_DELIM;

    /* Width of int in hex -- required for variable width format printing */
    const unsigned int long_hex_width = (unsigned int) sizeof(ssize_t) * 2;

    /* Bytes printed to buf */
    int b = 0;

    /* Parse item data */
    b = snprintf(buf, DIR_ITEM_ENTRY_LEN + 1,
                 "%0*zX%s%-*s%s",
                 /* Width and value of ID */
                 long_hex_width, itp->item_id,
                 delim,
                 ITEM_NAME_MAX, itp->item_name, /* Name */
                 term);

    /* Handle errors */
    if ((size_t) b < DIR_ITEM_ENTRY_LEN) {
        printf("Unable to save item, names must be less than %d characters\n",
               ITEM_NAME_MAX);
#ifdef DEBUG
        log_err("make_item_entry could not parse item data correctly");
#endif
    }
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
static int append_item_entry(const item *itp,
                            const char entry[DIR_ITEM_ENTRY_LEN + 1]) {
    int fd;
    int open_flags = O_APPEND | O_RDWR;

    switch (itp->item_st) {
    case TODO:
        fd = open(todo_path, open_flags);
        break;
    case IN_PROG:
        fd = open(ip_path, open_flags);
        break;
    case DONE:
        fd = open(done_path, open_flags);
        break;
#ifdef DEBUG
    default:
        log_err("Attempting to write item with an invalid state");
#endif
    }

    /* Avoid writing NULL-byte */
    if (write(fd, entry, DIR_ITEM_ENTRY_LEN) < 0)
        return -1;

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
 * @brief Find item offset in file opened by file descriptor fd provided its
 * ID
 * @param fd File descriptor of file representing item entries
 * @param id ID of item to find
 * @return offset of item in file represented by fd
 * @return -1 if item does not appear in fd
 */
static off_t fd_find_item_id(const int fd, const ssize_t id) {
    assert(fcntl(fd, F_GETFD) != -1); /* File descriptor is valid */

    int total_items = fd_total_items(fd);
    off_t off = -1;
    /* + 1 for character parsing */
    char curr_id_str[sizeof(ssize_t) * 2 + 1];

    for (int i = 0; i < total_items && off < 0; i++) {
        if (pread(fd, curr_id_str, sizeof(curr_id_str), i * DIR_ITEM_ENTRY_LEN)
            < 0) {
            return -1;
        }

        if (id == (ssize_t) strtoll(curr_id_str, NULL, 16)) /* Check ID */
            off = i * DIR_ITEM_ENTRY_LEN;
    }

    return off;
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
static int fd_swap_item_entries_at(const int fd, const off_t off_a,
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
    off_t total_item_size = fd_total_items(fd) * DIR_ITEM_ENTRY_LEN;
    /*
     * NOTE: NULL arg could produce undefined behaviour without previous call
     * to setup_path_names -- will be amended in future changes
     */

    if (off_a < 0 || off_a >= total_item_size ||
        off_b < 0 || off_b >= total_item_size) {
        return -1;
    }

    if (off_a == off_b) return 0;

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
 * @brief Remove item entry at given offset
 * @param entry_off Offset of entry
 * @param fd File descriptor of file of item entries in regular format
 * @return 0 on success
 * @return -1 on error or if file if opened by fd is empty
 * @note entry_off must be the offset of the *first* character of the item
 * entry
 */
static int fd_remove_item_entry_at(const off_t entry_off, const int fd) {
    assert(fcntl(fd, F_GETFD) != -1); /* File descriptor is valid */
    assert(entry_off >= 0);

    /* Swap with last entry */
    off_t total_file_size = fd_total_items(fd) * DIR_ITEM_ENTRY_LEN;
    if (entry_off >= total_file_size) {
#ifdef DEBUG
        log_err("Entry offset provided too large given item file size");
#endif
        return -1;
    }

    off_t last_entry_off = total_file_size - DIR_ITEM_ENTRY_LEN;

    /* Move entry to end of file */
    fd_swap_item_entries_at(fd, entry_off, last_entry_off);

    /* Truncate file */
    if (ftruncate(fd, last_entry_off) < 0) {
#ifdef DEBUG
        log_err("Item entry file could not be truncated when removing entry");
#endif
        return -1;
    }

    return 0;
}

/**
 * @brief Read item at an offset from fd as a new item struct
 * @param entry_off Offset of entry
 * @param fd File descriptor of file of item entries in regular format
 * @note entry_off must be the offset of the *first* character of the item
 * entry
 */
static item * fd_read_item_at(off_t entry_off, int fd) {
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

int dir_change_item_status_id(const ssize_t id, const enum status new_status) {
    assert(new_status < ITEM_STATUS_COUNT); /* Validate status */

    setup_path_names(NULL);

    int item_fds[_DIR_ITEM_NUM_FILES];

    item *itp = NULL;

    open_items(O_RDWR, item_fds);
    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        if (item_fds[i] < 0) {
#ifdef DEBUG
            log_err("Could not open item files for reading and writing");
#endif
            return -1;
        }
        /* Find item in project */
        off_t item_off = fd_find_item_id(item_fds[i], id); 

        if (item_off >= 0) {
            /* Status is already correct - exit from function */
            if ((enum status) i == new_status)
                break;

            int fd = item_fds[i];

            /* Remove item from current location */
            itp = fd_read_item_at(item_off, fd);

            if (!itp) { /* Could not read item */
                close_items(item_fds);
                return -1;
            }

            fd_remove_item_entry_at(item_off, fd);
        }
    }

    /* Item does not exist */
    if (!itp)
        return -1;

    /* Update status */
    itp->item_st = new_status;

    /* Add to new location */
    /*
     * FUTURE: Check *where* the item belongs in the file to preserve
     * order of IDs
     */
    dir_append_item(itp);

    close_items(item_fds);

    item_free(itp);

    return 0;
}
