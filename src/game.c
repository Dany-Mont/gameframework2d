#include <SDL.h>
#include "simple_logger.h"

#include "gf2d_graphics.h"
#include "gf2d_sprite.h"

#include "font.h"
#include "hud.h"
#include "camera.h"
#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "world.h"
#include "quadtree.h"
#include "bullet.h"
#include "worldgen.h"
#include "pickup.h"


int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    World *world;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color8(255,100,255,200);
    Entity *player;
    Quadtree *qt;
    
    Entity *activeEntities[1024];
    int entityCount = 0;
    int i;

    /*program initializtion*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    font_init();
    HUD hud;
    hud_init(&hud);
    entity_manager_init(1024);
    SDL_ShowCursor(SDL_DISABLE);
    camera_set_size(gfc_vector2d(1200,720));
    
    /*demo setup*/
    mouse = gf2d_sprite_load_all("../images/pointer.png",32,32,16,0);
    player = player_new();
    player_unlock_weapon(player, WEAPON_BLADE);
    player_unlock_weapon(player, WEAPON_CARDINAL);
    //player_unlock_weapon(player, WEAPON_LASER);
    //player_unlock_weapon(player, WEAPON_LIGHTNING);
    world = worldgen_generate(70, 90);
    //grunt_new(gfc_vector2d(400, 300));
    //grunt_new(gfc_vector2d(400, 300));
    //grunt_new(gfc_vector2d(400, 300));
    //grunt_new(gfc_vector2d(400, 300));
    //grunt_new(gfc_vector2d(400, 300));
    //grunt_new(gfc_vector2d(400, 300));
    //grunt_new(gfc_vector2d(400, 300));
    //tank_new(gfc_vector2d(1000, 200));
    //ranger_new(gfc_vector2d(1500, 400));
    //mage_new(gfc_vector2d(2700, 1300));
    //summoner_new(gfc_vector2d(1800, 1000));
    world_setup_camera(world);
    qt = quadtree_new(world);
    slog("press [escape] to quit");
    /*main game loop*/
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        font_cleanup();
        //slog("after font cleanup");
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        //slog("after mouse state");
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
        
        entity_manager_think_all();
        entity_manager_update_all();
        hud_update(&hud);

        /* --- Collision --- */
       entityCount = entity_manager_get_all_active(activeEntities, 1024);
        quadtree_clear(qt);
        for (int i = 0; i < entityCount; i++)
        {
            quadtree_insert(qt, activeEntities[i]);
        }
        quadtree_collide_all(qt, world, activeEntities, entityCount, player_on_entity_collide, entity_on_tile_collide);
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            world_draw(world);
            for (i = 0; i < entityCount; i++)
            {
                if (activeEntities[i] && activeEntities[i]->free == mage_free)
                    mage_draw_shield(activeEntities[i]);
            }
            if (player && player->_inuse)
            weapon_system_draw(&((PlayerData *)player->data)->weapons, player);
            hud_render(&hud, player_get_health(player), player_get_max_health(player));
            entity_draw_all(); //entities drawn after backgrounds
            // debug hitboxes — remove when done
            for (i = 0; i < entityCount; i++)quadtree_draw_entity_bounds(activeEntities[i], gfc_color8(255, 0, 0, 255));

            //UI elements last
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx,my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    entity_free(player);
    world_free(world);
    quadtree_free(qt);
    //slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
