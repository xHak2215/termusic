static struct termios oldt;

void enable_raw_mode() {
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // non-canonical, no echo
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    // make non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

typedef struct file_list {
    char**  list;
    size_t  num_files;
    size_t  max_len;
} Flist;

Flist lsdir(const char *path, const char *filter) {
    DIR *d = opendir(path);
    Flist list; 
    if (!d) return list;

    struct dirent *entry;
    size_t num_files = 0, max_len = 0, len_filter = strlen(filter);
    bool filt = false;

    list.list = NULL;
    list.max_len   = 0;
    list.num_files = 0;

    while ((entry = readdir(d)) != NULL) { 
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '\0' ||
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0')) ) 
            continue;

        size_t len = strlen(entry->d_name);
        
        filt = false;
        for (int i = len - 1, inx = len_filter - 1; i != len - len_filter; i--){
            //printf("i: %d inx: %d\n", i, inx);
            //printf("%s %c | %c\n", entry->d_name, entry->d_name[i], filter[inx]);
            if (entry->d_name[i] != filter[inx])
                break; 

            inx++;
            if (inx == len_filter)
                filt = true; 
        }
        if (!filt)
            continue;
        
        if (len > max_len)
            max_len = len;
    
        num_files++;
    }

    // Выделить массив указателей 
    char **file_list = malloc((num_files + 1) * sizeof(char *)); // num_files слишком большой вроде хз 
    if (!file_list) {
        closedir(d);
        return list;
    }

    // возврат в начало директории 
    rewinddir(d);

    size_t i = 0;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '\0' ||
             (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
            continue;
            
        size_t len = strlen(entry->d_name);
        
        filt = false;
        for (int i = len - 1, inx = len_filter - 1; i != len - len_filter; i--){
            if (entry->d_name[i] != filter[inx])
                break; 
            inx++;
            if (inx == len_filter)
                filt = true; 
        }
        if (!filt)
            continue;

        file_list[i] = strdup(entry->d_name);

        //if (!file_list[i]) {
        //    // при ошибке очистить ранее выделенное 
        //    for (size_t j = 0; j < i; j++) free(file_list[j]);
        //    free(file_list);
        //    closedir(d);
        //    return list;
        //1}
        i++;
    }

    file_list[num_files] = NULL;
    closedir(d);

    list.list = file_list;
    list.max_len = max_len;
    list.num_files = num_files;

    return list;
}

/*
char* get_formating_music_time(ma_sound* sound, ma_uint64 total_music_time){
    ma_format fmt = ma_sound_get_data_format(sound); 
    ma_uint32 sr = fmt.sampleRate;
    double seconds = (double)total_music_time / (double)sr;

    int hrs = (int)(seconds / 3600.0);
    int mins = (int)((seconds - hrs*3600.0) / 60.0);
    int secs = (int)(seconds - hrs*3600.0 - mins*60.0);

    char timestr[64];
    snprintf(timestr, sizeof(timestr), "%02d:%02d:%02d", hrs, mins, secs);
    return timestr;
}*/