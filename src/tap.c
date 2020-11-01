
#define _XOPEN_SOURCE_EXTENDE

#include <ncurses.h>
#include <locale.h>
#include <taglib/tag_c.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <unistd.h>

WINDOW *control_win;
WINDOW *queue_win;
WINDOW *music_win;

unsigned int current_music_index = 0;
bool quit = 0;
int volume = MIX_MAX_VOLUME / 2;

void set_volume(int n) {
  if (volume < MIX_MAX_VOLUME) {
    volume = volume + n;
  }
  wclear(control_win);
  wprintw(control_win, "Volume: %3d\n", volume);
  wrefresh(control_win);
  Mix_VolumeMusic(volume);
}

int main(int argc, char **argv)
{
  // INIT {{{
  setlocale(LC_ALL, "");

  SDL_Init(SDL_INIT_AUDIO);

  Mix_Init(MIX_INIT_MP3 | MIX_INIT_FLAC | MIX_INIT_OGG | MIX_INIT_OPUS);
  Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 4096);

  initscr();
  clear();
  noecho();
  cbreak();
  curs_set(0);

  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);

  // }}}

  int my, mx;
  getmaxyx(stdscr, my, mx);

  control_win = newwin(0, mx, my - 1, 1);
  wrefresh(control_win);

  queue_win = newwin(30, mx, 2, 0);
  wrefresh(queue_win);

  music_win = newwin(4, mx, 31, 1);
  wrefresh(music_win);

  scrollok(queue_win, TRUE);

  Mix_Music *queue[argc - 1];
  TagLib_Tag *tags[argc - 1];

  unsigned int num_of_musics = argc - 1;
  int ch;

  printw("No. Musics: %d\n", num_of_musics);
  refresh();

  wprintw(queue_win, "Queue:\n");

  // Data extraction
  for (size_t i = 0; i < num_of_musics; i++) {
    wprintw(queue_win, "%s [%d]\n", argv[i + 1], i);
    wrefresh(queue_win);
    queue[i] = Mix_LoadMUS(argv[i + 1]);
    tags[i] = taglib_file_tag(taglib_file_new(argv[i + 1]));
  }

  Mix_VolumeMusic(volume);

  while(!quit) {

    ch = getch();

    switch(ch) {
      case KEY_UP:
        set_volume(1);
        break;
      case KEY_DOWN:
        set_volume(-1);
        break;
      case KEY_F(1):
        quit = 1;
      default:
        break;
    }

    // Check if a music is playing
    if (!Mix_PlayingMusic()) {

      if (current_music_index != num_of_musics) {
        wclear(music_win);
        wprintw(music_win, "\nPlaying [%d]: %s\n", current_music_index, argv[current_music_index + 1]);
        wprintw(music_win, "Artist: %s\n", taglib_tag_artist(tags[current_music_index]));
        wprintw(music_win, "Album: %s\n", taglib_tag_album(tags[current_music_index]));
        wrefresh(music_win);

        Mix_PlayMusic(queue[current_music_index], 0);
      } else {
        quit = 1;
      }

      current_music_index++;
    }
  }

  // END {{{

  for (int i = 0; i < argc - 1; i++) {
    Mix_FreeMusic(queue[i]);
  }

  endwin();
  taglib_tag_free_strings();
  Mix_CloseAudio();
  Mix_Quit();
  SDL_Quit();

  // }}}
}
