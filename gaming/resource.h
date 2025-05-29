#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

typedef struct {
    ALLEGRO_BITMAP* carrier;//航母
    ALLEGRO_BITMAP* cruiser_light;
    ALLEGRO_BITMAP* cruiser_heavy;
    ALLEGRO_BITMAP* submarine;
    ALLEGRO_BITMAP* factory;
    ALLEGRO_BITMAP* radar;
    ALLEGRO_BITMAP* suicide_boat;
    ALLEGRO_BITMAP* attack_miss;
    ALLEGRO_BITMAP* attack_hit;
    ALLEGRO_BITMAP* attack_sunk;
} GameSprites;//將structt的型態叫做GameSprites，呼叫時直接GameSprites variable就好了，就會進行struct裡面的步驟

void load_resources(GameSprites* sprites);
void destroy_resources(GameSprites* sprites);