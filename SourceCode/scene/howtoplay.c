#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "howtoplay.h"
#include "scene.h"
#include "../global.h"
#include <math.h>

// 為滑動選項新增變數
int howtoplay_menu_index = 0;
const int howtoplay_menu_count = 2;

void howtoplay_update(Scene *self) {
    HowToPlay *obj = (HowToPlay *)self->pDerivedObj;
    int mx = mouse.x;
    int my = mouse.y;
    int bottom_y = HEIGHT - 50;

    // 按左右鍵切換焦點
    if (key_state[ALLEGRO_KEY_LEFT]) {
        key_state[ALLEGRO_KEY_LEFT] = false;
        howtoplay_menu_index = (howtoplay_menu_index - 1 + howtoplay_menu_count) % howtoplay_menu_count;
        al_play_sample(obj->select_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
    if (key_state[ALLEGRO_KEY_RIGHT]) {
        key_state[ALLEGRO_KEY_RIGHT] = false;
        howtoplay_menu_index = (howtoplay_menu_index + 1) % howtoplay_menu_count;
        al_play_sample(obj->select_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }

    // Enter 選擇目前選項
    if (key_state[ALLEGRO_KEY_ENTER]) {
        key_state[ALLEGRO_KEY_ENTER] = false;
        al_play_sample(obj->enter_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        al_rest(0.2);
        if (howtoplay_menu_index == 0) {
            window = 0; // Back
        } else {
            window = 1; // Start Game
        }
        self->scene_end = true;
    }

    // ESC 直接返回
    if (key_state[ALLEGRO_KEY_ESCAPE]) {
        key_state[ALLEGRO_KEY_ESCAPE] = false;
        al_play_sample(obj->select_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        al_rest(0.1);
        window = 0;
        self->scene_end = true;
    }
}

void howtoplay_draw(Scene *self) {
    HowToPlay *obj = (HowToPlay *)self->pDerivedObj;
    al_clear_to_color(al_map_rgb(0, 0, 0));

    if (obj->background_img) {
        al_draw_scaled_bitmap(obj->background_img,
                              0, 0,
                              al_get_bitmap_width(obj->background_img),
                              al_get_bitmap_height(obj->background_img),
                              0, 0,
                              WIDTH, HEIGHT, 0);
    }

    al_draw_text(obj->font, al_map_rgb(255, 255, 255),
                 WIDTH / 2, HEIGHT / 3,
                 ALLEGRO_ALIGN_CENTER, "How to Play");

    al_draw_text(obj->font, al_map_rgb(200, 200, 200),
                 WIDTH / 2, HEIGHT / 2,
                 ALLEGRO_ALIGN_CENTER, "This feature is under construction...");

    // Bottom navigation（相對位置改成跟畫面大小有關）
    int bottom_y = HEIGHT - HEIGHT / 20;
    int margin_x = fmin(80, WIDTH / 30);


    ALLEGRO_COLOR left_color = (howtoplay_menu_index == 0) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR right_color = (howtoplay_menu_index == 1) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);

    al_draw_text(obj->font, left_color, margin_x, bottom_y, 0, "← Back");
    al_draw_text(obj->font, right_color, WIDTH - margin_x, bottom_y, ALLEGRO_ALIGN_RIGHT, "Start Game →");
}

void howtoplay_destroy(Scene *self) {
    HowToPlay *obj = (HowToPlay *)self->pDerivedObj;
    al_destroy_font(obj->font);
    al_destroy_bitmap(obj->background_img);
    al_destroy_sample(obj->enter_sound);
    al_destroy_sample(obj->select_sound);
    free(obj);
    free(self);
}

Scene *New_HowToPlay(int label) {
    Scene *scene = New_Scene(label);
    HowToPlay *obj = (HowToPlay *)malloc(sizeof(HowToPlay));

    obj->font = al_load_ttf_font("assets/font/pixel.ttf", WIDTH / 40, 0);
    obj->background_img = al_load_bitmap("assets/image/howto_bg.png");
    obj->enter_sound = al_load_sample("assets/sound/enter.ogg");
    obj->select_sound = al_load_sample("assets/sound/select.wav");

    scene->pDerivedObj = obj;
    scene->Update = howtoplay_update;
    scene->Draw = howtoplay_draw;
    scene->Destroy = howtoplay_destroy;

    return scene;
}
