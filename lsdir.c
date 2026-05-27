#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

/*
char **lsdir(const char *path) {
    DIR *d = opendir(path);
    if (!d) return NULL;

    struct dirent *entry;
    size_t num_files = 0;
    size_t max_len = 0;

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '\0' ||
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
            continue;

        size_t len = strlen(entry->d_name);
        if (len > max_len) max_len = len;
        num_files++;
    }

    // Выделить массив указателей 
    char **file_list = malloc((num_files + 1) * sizeof(char *));
    if (!file_list) {
        closedir(d);
        return NULL;
    }

    // возврат в начало директории 
    rewinddir(d);

    size_t i = 0;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '\0' ||
             (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
            continue;

        file_list[i] = strdup(entry->d_name);
        if (!file_list[i]) {
            // при ошибке очистить ранее выделенное 
            for (size_t j = 0; j < i; j++) free(file_list[j]);
            free(file_list);
            closedir(d);
            return NULL;
        }
        i++;
    }

    file_list[i] = NULL;
    closedir(d);
    return file_list;
}
*/

int main(void) {
    char **list = lsdir(".");
    if (!list) {
        fprintf(stderr, "Failed to list directory\n");
        return 1;
    }

    for (size_t i = 0; list[i] != NULL; i++) {
        printf("%s\n", list[i]);
        free(list[i]); 
    }
    free(list); 

    return 0;
}
