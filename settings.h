enum { progress_bar_size = 15 }; // кол-во сигментов прогерсс бара
unsigned short cursor = 0; // позиция курсора по умаолчанию
short val = 100; // громкость по умолчанию
const char *extension_filter[] = {".mp3", ".wav", ".ogg", NULL}; // расширения файлы с котрыми будут в меню
ma_uint64 sampleRate = 48000; 
const short scrolling_music_factor = 10; // значение на котрое музыка перемотаеться аперед/назад (в сикундах)