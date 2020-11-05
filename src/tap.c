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
#include <libgen.h>

WINDOW *control_win;
WINDOW *queue_win;
WINDOW *music_win;

unsigned int music_index = 0;
unsigned int music_index_tmp = 0;
bool quit = FALSE;
int volume = MIX_MAX_VOLUME / 2;

void play_next(Mix_Music **queue)
{
  music_index++;
  Mix_PlayMusic(queue[music_index], 0);
}

void play_prev(Mix_Music **queue)
{
  music_index--;
  Mix_PlayMusic(queue[music_index], 0);
}

void show_music(TagLib_Tag **tags, char **argv)
{
  wclear(music_win);
  wprintw(music_win, "\nPlaying [%d]: %s\n", music_index, basename(argv[music_index + 1]));
  wprintw(music_win, "Artist: %s\n", taglib_tag_artist(tags[music_index]));
  wprintw(music_win, "Album: %s\n", taglib_tag_album(tags[music_index]));
  wrefresh(music_win);
}

void show_volume() {
  wclear(control_win);
  wprintw(control_win, "Volume: %3d\n", volume);
  wrefresh(control_win);
  Mix_VolumeMusic(volume);
}

void set_volume(unsigned int n) 
{
  if (volume > 0 && volume < MIX_MAX_VOLUME) {
    volume = volume + n;
    show_volume();
  }
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
  queue_win = newwin(my - 5, mx - 1, 0, 1);
  music_win = newwin(4, mx, my - 6, 2);

  wrefresh(control_win);
  wrefresh(music_win);
  scrollok(queue_win, TRUE);
  wrefresh(queue_win);

  refresh();

  Mix_Music *queue[argc - 1];
  TagLib_Tag *tags[argc - 1];

  unsigned int num_of_musics = argc - 1;
  int ch;

  // Data extraction
  for (size_t i = 0; i < num_of_musics; i++) {
    wprintw(queue_win, "[%d] %s\n", i, argv[i + 1]);
    queue[i] = Mix_LoadMUS(argv[i + 1]);
    tags[i] = taglib_file_tag(taglib_file_new(argv[i + 1]));
  }
  
  wrefresh(queue_win);

  Mix_VolumeMusic(volume);

  show_music(tags, argv);
  Mix_PlayMusic(queue[0], 0);

  while(!quit) {

    ch = getch();

    switch(ch) {
      case 'k':
        set_volume(1);
        break;
      case 'j':
        set_volume(-1);
        break;
      case 'h':
        if (music_index > 0) {
          music_index_tmp--;
        }
        break;
      case 'l':
        if (music_index < num_of_musics - 1) {
          music_index_tmp++;
        }
        break;
      case 'p':
        Mix_PauseMusic();
        break;
      case 'u':
        Mix_ResumeMusic();
        break;
      case 'q':
        quit = 1;
        break;
      default:
        break;
    }

    if (!Mix_PlayingMusic()) {
      if (music_index != num_of_musics) {
        music_index_tmp++;
      } else {
        quit = 1;
      }
    }

    if (music_index_tmp > music_index) {
      play_next(queue);
      show_music(tags, argv);

      music_index_tmp = music_index;
    }

    if (music_index_tmp < music_index) {
      play_prev(queue);
      show_music(tags, argv);

      music_index_tmp = music_index;
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
