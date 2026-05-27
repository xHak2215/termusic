#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>

#include "utils.h"
#include "miniaudio/miniaudio.c"

int main(void) {
    int cursor = 0;
    int val = 100;
    char* played = "none";
    char c[3];

    struct winsize w; // для размера терминала
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); // w.ws_row количество синволов; w.ws_col кол во колонок

    Flist list = lsdir(".", ".mp3");
    if (!list.list) {
        fprintf(stderr, "Failed to list directory\n");
        return 1;
    }
    char** file_list = list.list;
    size_t max_len = list.max_len;
    size_t files_num = list.num_files;

    ma_uint64 saved_cursor, total_music_time;
    ma_result result;
    ma_engine engine;
    ma_sound *sound = malloc(sizeof(ma_sound));

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "error init ma engine");
        return 1;
    }

    enable_raw_mode();
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds); // Очищаем и настраиваем множество дескрипторов
        FD_SET(STDIN_FILENO, &readfds);
        struct timeval tv = {0, 50000}; // 50 ms timeout — неблокирующее ожидание
        int rv = select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv); // следит за множествами файловых дескрипторов
        
        printf("\e[1;1H\e[2J");

        if (rv > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            ssize_t n = read(STDIN_FILENO, &c, 3);
            if (n > 0) {
                if ((size_t)n+1 >= 3){
                    // выбор трека 
                    if (c[0] == 0x1B && c[1] == '[' && c[2] == 'A'){
                        if (cursor > 0)
                            cursor--;
                    }
                    if (c[0] == 0x1B && c[1] == '[' && c[2] == 'B'){
                        if (cursor < files_num-1)
                            cursor++;
                    } 
                    // громкость звука
                    if ((c[0] == 0x1B && c[1] == '[' && c[2] == 'D') || c[0] == '+'){
                        if (val < 100)
                            val++;
                    }
                    if ((c[0] == 0x1B && c[1] == '[' && c[2] == 'C') || c[0] == '-'){
                        if (val > 0) 
                            val--;
                    }
                } else {
                    if (c[0] == '\n' || c[0] == '\r'){
                        played = file_list[cursor];
                        if (ma_sound_init_from_file(&engine, played, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, sound) != MA_SUCCESS) {
                            fprintf(stderr, "Failed to load sound\n");
                            free(sound);
                            ma_engine_uninit(&engine);
                            return 4;
                        }

                        ma_sound_start(sound);
                    }
                    if (c[0] == 'q') {
                        printf("\e[1;1H\e[2J");
                        return 0;
                    }
                }
                //printf("Key: %c%c%c (0x%02x)\n", c[0], c[1], c[2], (unsigned char)c[0]);
            }
        }
        
        printf("%ld files", files_num);
        for (size_t i = 0; file_list[i] != NULL; i++) {
            if (i < w.ws_col - 1){
                printf("\e[%d;%dH", (unsigned int)i+2, 1);
                //printf("\e[%zu;%dH", i+1, 1);
                if ((int)i == cursor) printf("> %s\n", file_list[i]);
                else printf("  %s\n", file_list[i]);
            }
            printf("\e[%d;%dH", w.ws_col-1, 1);
            printf("plays: %s | [---------------] val:%d", played, val);
            fflush(stdout);
            //usleep(5000);
        }
    }


    disable_raw_mode();
    ma_engine_uninit(&engine);
    for (size_t i=0; file_list[i]; i++) free(file_list[i]);
    free(file_list);
    return 0;
}
