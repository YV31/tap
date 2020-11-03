/*       __            
 *      / /_____ _____ 
 *     / __/ __ `/ __ \
 *    / /_/ /_/ / /_/ /
 *    \__/\__,_/ .___/ 
 *            /_/
 *
 *  Terminal Audio Player
 */

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

unsigned int music_index = 0;
bool quit = FALSE;
int volume = MIX_MAX_VOLUME / 2;

void show_index()
{
  wclear(control_win);
  wprintw(control_win, "Index: %3d\n", music_index);
  wrefresh(control_win);
}

void show_music(TagLib_Tag **tags, char **argv)
{
  wclear(music_win);
  wprintw(music_win, "\nPlaying [%d]: %s\n", music_index, argv[music_index + 1]);
  wprintw(music_win, "Artist: %s\n", taglib_tag_artist(tags[music_index]));
  wprintw(music_win, "Album: %s\n", taglib_tag_album(tags[music_index]));
  wrefresh(music_win);
}

void set_volume(int n) 
{
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
  queue_win = newwin(my - 6, mx - 2, 0, 1);
  music_win = newwin(4, mx, my - 6, 2);

  wrefresh(control_win);
  wrefresh(queue_win);
  scrollok(queue_win, TRUE);
  wrefresh(music_win);

  refresh();

  Mix_Music *queue[argc - 1];
  TagLib_Tag *tags[argc - 1];

  unsigned int num_of_musics = argc - 1;
  int ch;

  // Data extraction
  for (size_t i = 0; i < num_of_musics; i++) {
    wmove(queue_win, i + 1, 1);
    wprintw(queue_win, "[%d] %s\n", i, argv[i + 1]);
    queue[i] = Mix_LoadMUS(argv[i + 1]);
    tags[i] = taglib_file_tag(taglib_file_new(argv[i + 1]));
  }
  
  box(queue_win, 0, 0);
  
  wmove(queue_win, 0, 1);
  wprintw(queue_win, " Queue ");

  wrefresh(queue_win);

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
      case KEY_LEFT:
        music_index--;
        show_music(tags, argv);
        show_index();
        Mix_PlayMusic(queue[music_index], 0);
        break;
      case KEY_RIGHT:
        music_index++;
        show_music(tags, argv);
        show_index();
        Mix_PlayMusic(queue[music_index], 0);
        break;
      case KEY_F(1):
        quit = 1;
        break;
      default:
        break;
    }

    // Check if a music is playing
    if (!Mix_PlayingMusic()) {

      if (music_index != num_of_musics) {
        wclear(music_win);
        wprintw(music_win, "\nPlaying [%d]: %s\n", music_index, argv[music_index + 1]);
        wprintw(music_win, "Artist: %s\n", taglib_tag_artist(tags[music_index]));
        wprintw(music_win, "Album: %s\n", taglib_tag_album(tags[music_index]));
        wrefresh(music_win);

        Mix_PlayMusic(queue[music_index], 0);
      } else {
        quit = 1;
      }

      music_index++;
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
