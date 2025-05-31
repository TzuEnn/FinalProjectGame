#include "sceneManager.h"
#include "menu.h"
#include "gamescene.h"
#include "howtoplay.h"  

Scene *scene = NULL;

void create_scene(SceneType type)
{
    switch (type)
    {
    case Menu_L:
        scene = New_Menu(Menu_L);
        break;
    case GameScene_L:
        scene = New_GameScene(GameScene_L);
        break;
    case HowToPlay_L:                         
        scene = New_HowToPlay(HowToPlay_L);
        break;
    default:
        break;
    }
}
