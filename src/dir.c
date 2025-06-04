#include "dir.h"
#include "config.h"
#include "ds/item.h"
#include "tojo.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

static char item_path[_MAX_PATH];

/**
 * @brief creates items file, file descriptor is closed immediately if creation
 * is successful
 * @return 0 on success, -1 on error
 * @see open, close
 */
static int create_items(const char *path) {
    dir_construct_path(path, _DIR_ITEM_PATHF, item_path, _MAX_PATH);

    /* Create file as a contained resource */
    /* Plain text file */
    int retfd = open(item_path, O_CREAT, CONF_DIR_PERMS & 0666);
    if (retfd == -1) {
#ifdef DEBUG
        log_err("Could not create items file");
#endif
        /* Function should only be called on new projects */
        assert(errno != EEXIST);
        return -1;
    }
    close(retfd);

    return 0;
}

/**
 * @brief Open items file in project, static item_path must be set correctly
 * @param flags open flags
 * @see open
 */
static int open_items(int flags) {
    assert(strlen(item_path) >= sizeof(CONF_PROJ_DIR) + sizeof(_DIR_ITEM_PATHF)
           - 1);
    return open(item_path, flags);
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

    /* Items */
    ret = create_items(path);

    return ret;
}

/**
 * @brief Read item name from file opened by items_fd
 * @return Pointer to allocated item, NULL if no matching item is found
 * @see item_init, item_free
 */
static item * read_item_name(const char *name, const int items_fd) {
    assert(name); /* Name exists */
    assert(items_fd > 0); /* Valid file descriptor */
    /* Has read perms */
    assert((fcntl(items_fd, F_GETFL) & O_ACCMODE) != O_WRONLY);

    size_t name_len = strlen(name);

    /* Read item from file */
    /* Assumes that each line contains only the item name */
    char buf[CONF_ITEM_NAME_SZ];
    char *next_name;
    int bytes;

    /* Read lines */
    while ((bytes = read(items_fd, buf, sizeof(buf) - 1)) > 0) {
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
    int items_fd = open_items(O_RDONLY);

    if (items_fd < 0) {
#ifdef DEBUG
        log_err("Could not open items for reading");
#endif
        return NULL;
    }

    item *itp = read_item_name(name, items_fd);
    close(items_fd);

#ifdef DEBUG
    if (itp == NULL) {
        log_err("Item could not be allocated");
    }
#endif

    return itp;
}

int dir_number_of_items(const char *path) {
    if (!*item_path) {
        dir_construct_path(path, _DIR_ITEM_PATHF, item_path, _MAX_PATH);
    }

    int items_fd = open_items(O_RDONLY);

    if (items_fd < 0) {
        return items_fd;
    }

    char buf[BUFSIZ];
    int num_items = 0;
    size_t bytes;

    while ((bytes = read(items_fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';
        /* Count delimiters found in buffered read */
        const char *ptr = buf;
        while ((ptr = strchr(ptr, *_DIR_ITEM_DELIM))) {
            num_items++;
            ptr++;
        }
    }

    close(items_fd);

    return num_items;
}

item ** dir_get_all_items(const char *path) {
    assert(path);
    
    /* Construct item path */
    dir_construct_path(path, _DIR_ITEM_PATHF, item_path, _MAX_PATH);

    item **items = (item **) malloc(sizeof(item *)
                                    * (dir_number_of_items(NULL) + 1));

    if (!items) {
        puts("Could not read any items");
#ifdef DEBUG
        log_err("dir_get_all_items: malloc call failed");
#endif
        return NULL;
    }
    
    unsigned int num_items = 0;

    int items_fd = open_items(O_RDONLY);

    if (items_fd < 0)
        return items;

    char buf[CONF_ITEM_NAME_SZ];
    char *next_name;
    int bytes;

    /* Read lines */
    while ((bytes = read(items_fd, buf, sizeof(buf) - 1)) > 0) {
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

#ifdef DEBUG
    if (bytes == -1)
        log_err("Error when reading items");
#endif

    items[num_items] = NULL;

    return items;
}
/**
 * @brief Write the item name of the item pointed to by itp to the items_fd
 */
static int write_item_name(const item *itp, const int items_fd) {
    return write(items_fd, itp->item_name, strlen(itp->item_name));
}

int dir_write_item(const item *it, const char *path) {
    assert(it != NULL);

    /* Make path and write item */
    dir_construct_path(path, _DIR_ITEM_PATHF, item_path, _MAX_PATH);

    int items_fd = open_items(O_APPEND | O_RDWR); /* Append writing */

    if (items_fd < 0)
        return -1;

    /* Write item data to items file */
    write_item_name(it, items_fd);

    write(items_fd, _DIR_ITEM_DELIM, sizeof(_DIR_ITEM_DELIM) - 1);
    
    if (close(items_fd) == -1)
        return -1;

    return 0;
}
