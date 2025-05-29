#ifndef HOWTOPLAY_H_INCLUDED
#define HOWTOPLAY_H_INCLUDED

#include "scene.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>

typedef struct _HowToPlay {
    ALLEGRO_FONT *font;                      // 字體
    ALLEGRO_BITMAP *background_img;         // 背景圖
    ALLEGRO_SAMPLE *enter_sound;            // 點擊音效
    ALLEGRO_SAMPLE *select_sound;

} HowToPlay;

Scene *New_HowToPlay(int label);

#endif
