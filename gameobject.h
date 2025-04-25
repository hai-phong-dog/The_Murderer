#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include<algorithm>
#include<SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include<vector>

struct bullet {
    int damage = 10;
    int speed = 20;
    SDL_Texture* texture = nullptr;
    SDL_Rect srcRect, dstRect;
    double bullet_angle = 0;
    bool isdead = false;
    bullet(SDL_Renderer* renderer,int damage, int speed, const char* path, int x, int y, int w, int h, double angle);
    destroy();
};

struct melee {
    int weapon_type = 0;
    Mix_Chunk* attack_sound;
    SDL_Rect hitbox = {0, 0, 0, 0};
    int damage = 0;
    int attack_duration = 200;
    int cool_down_duration = 400;
    int range = 0;
    melee() = default;
    melee(int width, int height, int damage,int range,int attack_duration, int cool_down_duration,char* attack_soundd,int weapon_type);
};

struct complex_texture
{
    SDL_Texture* texture = nullptr;
    int numofframe = 1;
};


struct GameObject{
std::vector<complex_texture> textures;
int texture_index = 0;
SDL_Texture* texture;
SDL_Rect srcRect,dstRect;
int objectspeed;
int objectspeed2;
int objecthp;
double objectangle;
int object_type = 0;

//for animations and actions handle;
bool isdead = false;
bool go_through_able = false;//for intact object only;

bool isrunning = false;
int numofframe;
int currentframe = 0;
int lastframetime = SDL_GetTicks();
int currentframetime = lastframetime;
Uint16 walking_duration;

melee shortrange_weapon;
Uint32 lastattacktime = 0;
bool isreadyforattack = true;
bool isattacking = false;

Mix_Chunk* dead_sound;
bool played_dead_sound = false;
bool played_attack_sound = false;




void anothertexture();
void emplace_backtexture(SDL_Renderer* renderer, const char* texturePath, int numofframe);



//no touching here;

GameObject(SDL_Renderer* renderer,const char* texturePath,const char* d_sound,int x,int y,int w,int h,int objectspeed,int objecthp,double objectangle,int numofframe,Uint16 walking_duration,int object_type);
~GameObject();
void setSourceRect(int x,int y,int w,int h);
void setDestinationRect(int x,int y,int w,int h);
void betterrender(double angle,SDL_Renderer* renderer);
void check_if_died();
void move(int dx,int dy);

void meleeattack();
void calculate_melee_hitbox();
};


bool checkcollision(const SDL_Rect& A,const SDL_Rect& B);

void follow(GameObject& follower,GameObject& target);
void escape1(GameObject& escaper,GameObject& follower,int bgwidth,int bgheight);
void escape2(SDL_Rect &a,SDL_Rect &b,int vol);
void smartEscape(SDL_Rect &a,const SDL_Rect &b,int vol);
int clamp(int value, int min_value, int max_value);
Uint32 getPixel(SDL_Surface* surface, int x, int y);
bool checkPerPixelCollision(SDL_Surface* spriteA, SDL_Rect rectA,SDL_Surface* spriteB, SDL_Rect rectB);


void updateCamera(SDL_Rect &camera,GameObject &player,int SCREEN_WIDTH,int SCREEN_HEIGHT,int WORLD_WIDTH,int WORLD_HEIGHT);
void rendercopytocamera(SDL_Renderer* renderer,SDL_Rect camera,std::vector<GameObject*>& gameobjects);
void rendercopytocamera_for_alive(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects);
void rendercopytocamera_for_dead(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects);
void rendercopytocamera_for_bullet(SDL_Renderer* renderer, SDL_Rect camera, std::vector<bullet*>& bullets);
void Initializer(SDL_Window* &window,SDL_Renderer* &renderer,int SCREEN_WIDTH,int SCREEN_HEIGHT);
void EndEverything(SDL_Window* &window,SDL_Renderer* &renderer);




void check_if_moved(GameObject& moving_things, SDL_Rect& temprect);
void check_attack_and_calculate_damage(const GameObject& attacker,std::vector<GameObject*>& Other_object);
void check_attack_and_calculate_damage_for_a_and_b(const GameObject& attacker,GameObject& victim);
bool check_if_collided(const GameObject& moving_things,const std::vector<GameObject*>& Other_object);
bool check_if_collided_plus_physic_1(GameObject& moving_things,const std::vector<GameObject*>& Other_object);
void greater_check_if_died(std::vector<GameObject*> &objects,int map_x,int map_y);
void ObjectKiller(std::vector<GameObject*> &objects);

struct cell {
    int x, y;
    int cost;
    SDL_Point flowVector;

    cell(); // Constructor
};
bool isFull(std::vector<std::vector<cell>>& matrix,int default_value);
void computeCostField(std::vector<std::vector<cell>>& grid);
void drawVector(std::vector<std::vector<cell>>& arr);
void initialcostfield(std::vector<std::vector<cell>>& grid,const std::vector<GameObject*>other_objects,int cellsize);
bool isValid(int x, int y, int sizex, int sizey, const std::vector<std::vector<cell>>& arr);
void drawbetterVector(std::vector<std::vector<cell>>& arr);
void follow_the_vector(GameObject& npc,const std::vector<std::vector<cell>>& grid, int cellsize);

void random_npcs_generator(SDL_Renderer* renderer, std::vector<GameObject*>& moving_things,const std::vector<std::vector<cell>>& map_grid,int mapsize_x,int mapsize_y,int cell_size);
int count_dead_things(const std::vector<GameObject*>& moving_things);

void handleMovement(GameObject& object, int speed);
void angle_a_to_b(GameObject &a,GameObject &b);
void angle_otom(GameObject &player,const SDL_Rect &camera);
void angle_to_player(SDL_Rect& tempdstRect,GameObject& npc,GameObject& player);
void angle_otomovement(SDL_Rect& tempdstRect,GameObject& npc);
void physics_1(GameObject& a, GameObject& b);
void random_movement(GameObject& a);
//the ultimate love for you;
void the_ultimate_status_handler_for_things(std::vector<GameObject*>other_objects,std::vector<GameObject*>npcs,GameObject& player);
void the_ultimate_movement_and_status_handler(SDL_Renderer* renderer,std::vector<GameObject*>npcs,const std::vector<std::vector<cell>>& map_grid,int cellsize,std::vector<GameObject*>other_objects,GameObject& player,std::vector<bullet*>& bullets);
void the_ultimate_movement_and_status_handler_for_player(SDL_Renderer* renderer,GameObject& player,const SDL_Rect& camera,std::vector<GameObject*>other_objects,std::vector<GameObject*> npcs,std::vector<bullet*>& bullets);
void the_ultimate_animation_handler(GameObject& moving_things,int frame_duration_for_walking_texture);
void the_more_ultimate_animation_handler(std::vector<GameObject*>moving_things);
void the_ultimate_sound_effects_handler(std::vector<GameObject*>moving_things);
void the_ultimate_sound_effects_handler_for_single_object(GameObject& moving_thing);
void handle_movement_of_bullet(std::vector<bullet*>& bullets);
void handle_collision_of_bullet(std::vector<bullet*>& bullets,std::vector<GameObject*> things);
void handle_status_and_delete_dead_bullets(std::vector<bullet*>& bullets,int map_x,int map_y);


bool checkClick(SDL_Rect button, int x, int y);
void game();


#endif // GAMEOBJECT_H
