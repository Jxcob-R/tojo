#include "dir.h"
#include "config.h"
#include "ds/item.h"
#include "tojo.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

static char items_path[_MAX_PATH];
static char todo_path[_MAX_PATH];
static char ip_path[_MAX_PATH];
static char done_path[_MAX_PATH];

/**
 * @brief Set up global path variables to default values if not already done
 * so. Should be one of the first calls made by dir_* functions (excluding) for
 * project initialisation.
 * @param path Path to project directory
 * @note Modifies global variables, should be used once in a dir API call
 */
static void setup_path_names(const char *const path) {
    /* Set up items path */
    if (!*items_path)
        dir_construct_path(path, _DIR_ITEM_PATH_D, items_path, _MAX_PATH);

    /* Item file names */
    if (!*todo_path)
        dir_construct_path(items_path, _DIR_ITEM_TODO_F, todo_path, _MAX_PATH);
    if (!*ip_path)
        dir_construct_path(items_path, _DIR_ITEM_INPROG_F, ip_path, _MAX_PATH);
    if (!*done_path)
        dir_construct_path(items_path, _DIR_ITEM_DONE_F, done_path, _MAX_PATH);

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
static int create_items(const char *const path) {
    dir_construct_path(path, _DIR_ITEM_PATH_D, items_path, _MAX_PATH);

    assert(items_path != NULL);

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
    int file_creation;
    file_creation = create_file(todo_path)
                    + create_file(ip_path)
                    + create_file(done_path);

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

        return 0;
    }
    
    /* No project found */
    dir[0] = '\0';
    return -1;
}

void dir_construct_path(const char *path, const char *base, char *buf,
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
    ret = create_items(path);

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
    char buf[CONF_ITEM_NAME_SZ];
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
                item_set_name_deep(itp, name);
                return itp;
            }
            next_name = strtok(NULL, "\n");
        }
    }

    return NULL;
}

item * dir_find_item(const char *name, const char *path) {
    assert(path);
    
    setup_path_names(path);

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
 * @brief Count items in file descriptor fd
 * @pararm fd File descriptor to read, assumed to contain item data in required
 * format
 * @note To abstract buffered reading to separate function TODO:
 */
static int fd_count_items(const int fd) {
    char buf[BUFSIZ];
    int num_items = 0;
    ssize_t bytes;
    
    while ((bytes = read(fd, buf, sizeof(buf) - 1)) > 0) {
        if (bytes < 0)
            return bytes; /* Return error */

        buf[bytes] = '\0';
        /* Count delimiters found in buffered read */
        const char *ptr = buf;
        while ((ptr = strchr(ptr, *_DIR_ITEM_DELIM))) {
            num_items++;
            ptr++;
        }
    }
    return num_items;
}

int dir_number_of_items(const char *const path) {
    setup_path_names(path);

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

    /*
     * Buffered read of items
     * All file descriptors are read meaning that all items (whether complete
     * or outstanding) are counted
     */
    int file_count; /* Scratch variable */
    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        file_count = fd_count_items(item_fds[i]);
        if (file_count < 0)
            return file_count;
        num_items += file_count;
    }

    close_items(item_fds);

    return num_items;
}

item ** dir_get_all_items(const char *const path) {
    assert(path);
    
    /* Construct item path */
    dir_construct_path(path, _DIR_ITEM_PATH_D, items_path, _MAX_PATH);

    item **items = (item **) malloc(sizeof(item *)
                                    * (dir_number_of_items(NULL) + 1));

    if (!items) {
        puts("Could not read any items");
#ifdef DEBUG
        log_err("dir_get_all_items: malloc call failed, check item entries");
#endif
        return NULL;
    }
    
    int item_fds[_DIR_ITEM_NUM_FILES];

    open_items(O_RDONLY, item_fds);

    /* Simple opening check */
    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++)
        if (item_fds[i] < 0)
            return NULL;

    unsigned int num_items = 0;

    char buf[CONF_ITEM_NAME_SZ];
    char *next_name;
    ssize_t bytes;

    /* Read lines */
    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        while ((bytes = read(item_fds[i], buf, sizeof(buf) - 1)) > 0) {
            if (bytes < 0)
                return NULL;

            buf[bytes] = '\0';

            /* Extract Names */
            next_name = strtok(buf, _DIR_ITEM_DELIM);

            while (next_name != NULL) {
                /* Add new initialised item */
                items[num_items] = item_init();
                item_set_name_deep(items[num_items], next_name);
                num_items++;
                next_name = strtok(NULL, _DIR_ITEM_DELIM);
            }
        }
    }

#ifdef DEBUG
    if (bytes == -1)
        log_err("Error when reading items");
#endif

    /* Set last item pointer to NULL */
    items[num_items] = NULL;

    return items;
}

/**
 * @brief Write the item name of the item pointed to by itp to to the
 * appropriate items file
 * @param itp Pointer to item to write data of
 * @return 0 on success
 * @return -1 on error
 */
static int write_item_name(const item *itp) {
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

    /* Current write involves no padding */
    /* TODO: Add padding space on write */
    if (write(fd, itp->item_name, strlen(itp->item_name)) < 0)
        return -1;

    if (write(fd, _DIR_ITEM_DELIM, sizeof(_DIR_ITEM_DELIM)) < 0)
        return -1;

    return 0;
}

int dir_write_item(const item *it, const char *path) {
    assert(it != NULL);

    setup_path_names(path);

    int item_fds[_DIR_ITEM_NUM_FILES];
    open_items(O_APPEND | O_RDWR, item_fds);

    for (int i = 0; i < _DIR_ITEM_NUM_FILES; i++) {
        if (item_fds[i] < 0) {
#ifdef DEBUG
            log_err("Could not open item file for reading");
#endif
            return -1;
        }
    }

    /* Write item data to items file */
    if (write_item_name(it) < 0)
        return -1;
    
    close_items(item_fds);
    return 0;
}
