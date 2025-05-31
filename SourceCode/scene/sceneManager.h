#ifndef SCENEMANAGER_H_INCLUDED
#define SCENEMANAGER_H_INCLUDED
#include "scene.h"
extern Scene *scene;
typedef enum SceneType 
{
    Menu_L = 0,
    GameScene_L = 1,
    HowToPlay_L = 2
} SceneType;

void create_scene(SceneType);

#endif