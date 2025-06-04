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
                                    * dir_number_of_items(NULL));
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

    memset(items + num_items, 0, sizeof(item));

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
