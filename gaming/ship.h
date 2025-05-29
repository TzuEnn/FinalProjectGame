#pragma once
#include <stdbool.h>

#define GRID_SIZE 30
#define MAX_SHIPS 20

typedef struct GameState GameState;

typedef enum {
    SHIP_FACTORY,
    SHIP_CARRIER,
    SHIP_CRUISER_LIGHT,
    SHIP_CRUISER_HEAVY,
    SHIP_RADAR,
    SHIP_SUBMARINE,
    SHIP_SUICIDE_BOAT
} ShipType;

typedef struct {
    ShipType type;
    int health;
    int pos_x, pos_y;
    int cooldown;
    bool is_radar_active;
    bool can_move_in_radar; // 新增：標記船隻是否可在雷達範圍內移動
    int size;  // 船艦佔用格數
    bool is_visible; // 新增：自殺快艇可見性控制
    bool is_horizontal; // 新增方向標誌
} Ship;

struct GameState;

Ship create_ship(ShipType type, int x, int y);
void update_cooldowns(Ship* fleet);
void factory_production(GameState* game, bool is_player, int production_choice);
bool can_factory_produce(GameState* game, bool is_player);
bool is_position_valid_for_ship(GameState* game, int x, int y, int size, bool is_player, ShipType type);
int find_empty_ship_slot(Ship* fleet);