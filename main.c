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
#include <math.h>

#include "miniaudio/miniaudio.c"
#include "utils.h"

#define PROGRESS_BAR_SIZE 15

int main(void) {
    int cursor = 0;
    int val = 100;
    char* played = "none";
    bool pause = false;
    char c[3];

    struct winsize w; // для размера терминалаb
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); // w.ws_row количество синволов; w.ws_col кол во колонок

    Flist list = lsdir(".", ".mp3");
    if (!list.list) {
        fprintf(stderr, "Failed to list directory\n");
        return 1;
    }
    char** file_list = list.list;
    size_t max_len = list.max_len;
    size_t files_num = list.num_files;

    ma_uint64 saved_cursor = 0, total_music_time = 0;
    ma_result result;
    ma_engine engine;
    ma_sound sound;
    double secund = 0, total_music_time_second = 0;
    ma_uint64 sampleRate = 48000; // это затычка, поже нужно сделать получение реального 

    char progress_bar[PROGRESS_BAR_SIZE] = "               ";
    short percent = 0;

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "\nerror init ma engine [%i]\n", result);
        return 1;
    }

    int start_music() {
        if (played != "none") { 
            ma_sound_stop(&sound);
            ma_sound_set_start_time_in_pcm_frames(&sound, 0); // при запуске другого трека переводимся на 0
            ma_sound_uninit(&sound);
        }

        //ma_engine_config engineConfig = ma_engine_config_init();
        //sampleRate = engineConfig.sampleRate;

        played = file_list[cursor];

        if (ma_sound_init_from_file(&engine, played, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC, NULL, NULL, &sound) != MA_SUCCESS) {
            fprintf(stderr, "\nFailed to load sound\n");
            ma_engine_uninit(&engine);
            return 4;
        }

        result = ma_sound_get_length_in_pcm_frames(&sound, &total_music_time);
        total_music_time_second = (double)total_music_time / (double)sampleRate;
        if (result != MA_SUCCESS) {
            fprintf(stderr, "\nFailed to get total frames music [%i]\n", result);
            return 1;
        }
        // обнуление перед воспроизведением

        for (size_t i = 0; i < PROGRESS_BAR_SIZE; i++){
            progress_bar[i] = ' ';
        }
        percent = 0;
        secund = 0;
        saved_cursor = 0;

        ma_sound_set_volume(&sound, (val / 0.1) / 1000);
        ma_sound_start(&sound);
    }

    enable_raw_mode();

    printf("\e[1;1H\e[2J");  

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds); // Очищаем и настраиваем множество дескрипторов
        FD_SET(STDIN_FILENO, &readfds);
        struct timeval tv = {0, 50000}; // 50 ms timeout — неблокирующее ожидание
        int rv = select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv); // следит за множествами файловых дескрипторов

        if (total_music_time != 0){
            ma_sound_get_cursor_in_pcm_frames(&sound, &saved_cursor);
            secund = ((double)saved_cursor / (double)sampleRate);
        }

        if (rv > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            ssize_t n = read(STDIN_FILENO, &c, 3);
            if (n > 0) {
                if (c[0] == EOF) { usleep(10000); continue; }
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
                        if (played != "none")
                            ma_sound_set_volume(&sound, (val / 0.1) / 1000);
                    }
                    if ((c[0] == 0x1B && c[1] == '[' && c[2] == 'C') || c[0] == '-'){
                        if (val > 0) 
                            val--;
                        if (played != "none")
                            ma_sound_set_volume(&sound, (val / 0.1) / 1000);
                    }
                } else {
                    if (c[0] == '\n' || c[0] == '\r'){
                        start_music();
                    }
                    if (c[0] == 'q') {
                        printf("\e[1;1H\e[2J");
                        break;
                    }
                    if (c[0] == ' ') {
                        ma_sound_set_start_time_in_pcm_frames(&sound, saved_cursor);

                        if (pause){
                            ma_sound_start(&sound);
                            pause = false;
                        } else {
                            ma_sound_stop(&sound);
                            pause = true;
                        }
                    }
                }
                //printf("Key: %c%c%c (0x%02x)\n", c[0], c[1], c[2], (unsigned char)c[0]);
            }
        }
        char *pause_message = "playing";

        for (size_t i = 0; i <= w.ws_col; i++){
            printf("\e[%d;%ldH ", w.ws_row, i);
        }

        if (pause && played != "none"){
            pause_message = "pause";
        } else if (played != "none"){  
            pause_message = "playing";
        } else {
            pause_message = "";
        }

        printf("\e[%d;%dH", 0, 0);
        printf("%ld files\n", files_num);

        for (size_t i = 0; file_list[i] != NULL; i++) {
            if (i < w.ws_col - 1){
                if (i == cursor) printf("> %s\n", file_list[i]);
                else printf("  %s\n", file_list[i]);
            }
            //printf("\e[%d;%dH", w.ws_col-1, 1);
        }

        if (played != "none") { 
            percent = (((double)secund / (double)total_music_time_second) * 100);
            if (percent > 100) percent = 100;
            if (percent < 0) percent = 0; 
            unsigned short prog = (unsigned short)round(0.15 * percent);

            //for (size_t i = 0; i < prog; i++) {
            //    progress_bar[i] = '-';
            //}
            progress_bar[prog] = '-';
        }

        if (secund >= total_music_time_second && total_music_time_second > 0 && secund > 0) {
            if (cursor < files_num-1 && cursor != files_num-1){
                cursor++;
                start_music();
            } else {
                ma_sound_stop(&sound);
            }
        }

        printf("\e[%d;%dH", w.ws_row, 0);
        printf("plays: %s | %.1f [%s] val:%d        %s", played, secund, progress_bar, val, pause_message);
        
        fflush(stdout);
          
    }


    disable_raw_mode();
    ma_engine_uninit(&engine);
    for (size_t i=0; file_list[i]; i++) free(file_list[i]);
    free(file_list);
    return 0;
}
