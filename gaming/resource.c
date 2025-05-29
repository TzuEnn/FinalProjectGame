#include "resource.h"
#include <stdio.h>
#include "log.h"
#include <stdlib.h>

//放照片用
void load_resources(GameSprites* sprites) {
    log_message("[调试] 当前工作目录：%s\n", al_get_current_directory());
    // 加载 carrier.png
    sprites->carrier = al_load_bitmap("assets/carrier.png");
    if (!sprites->carrier) {
        log_message("[錯誤] 加載失敗！錯誤代碼: %d, Allegro錯誤: %s\n", al_get_errno(), al_get_errno() == ENOENT ? "文件不存在" : "其他錯誤");
        exit(1);
    }
    sprites->cruiser_light = al_load_bitmap("assets/cruiser_light.png");
    sprites->cruiser_heavy = al_load_bitmap("assets/cruiser_heavy.png");
    sprites->submarine = al_load_bitmap("assets/submarine.png");
    sprites->factory = al_load_bitmap("assets/factory.png");
    sprites->radar = al_load_bitmap("assets/radar.png");
    sprites->suicide_boat = al_load_bitmap("assets/suicide_boat.png");
    sprites->attack_miss = al_load_bitmap("assets/attack_miss.png");
    sprites->attack_hit = al_load_bitmap("assets/attack_hit.png");
    sprites->attack_sunk = al_load_bitmap("assets/attack_sunk.png");
}

void destroy_resources(GameSprites* sprites) {
    al_destroy_bitmap(sprites->carrier);
    al_destroy_bitmap(sprites->cruiser_light);
    al_destroy_bitmap(sprites->cruiser_heavy);
    al_destroy_bitmap(sprites->submarine);
    al_destroy_bitmap(sprites->factory);
    al_destroy_bitmap(sprites->radar);
    al_destroy_bitmap(sprites->suicide_boat);
}