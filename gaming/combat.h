
#pragma once
#include "game.h"

void carrier_attack(GameState* game, bool is_player);
void light_cruiser_attack(GameState* game, int x, int y);
void heavy_cruiser_attack(GameState* game, int x, int y, bool horizontal);
void suicide_boat_attack(GameState* game, int target_x, int target_y, int boat_index);
void submarine_attack(GameState* game, bool is_player);
void radar_ship_effect(GameState* game, Ship* radar_ship);
void handle_radar_damage(GameState* game, int attack_x, int attack_y);
void mark_ships_for_radar_movement(GameState* game, bool is_player_radar);
void clear_radar_map(GameState* game);
void normal_attack(GameState* game, int x, int y);