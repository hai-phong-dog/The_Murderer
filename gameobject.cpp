#include "gameobject.h"
#include<SDL_image.h>
#include<SDL2/SDL.h>
#include<bits/stdc++.h>


bullet::bullet(SDL_Renderer* renderer,int damage, int speed, const char* path, int x, int y, int w, int h, double angle)
    : damage(damage), speed(speed), bullet_angle(angle) {
    texture = IMG_LoadTexture(renderer,path);
    srcRect = {0, 0, w, h};
    dstRect = {x, y, w, h};
}
bullet::destroy() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }

melee::melee(int width, int height, int damage,int range,int attack_duration, int cool_down_duration,char* attack_soundd,int weapon_type){
    this->weapon_type = weapon_type;
    this->hitbox.w = width;
    this->hitbox.h = height;
    this->damage = damage;
    this->range = range;
    this->attack_duration = attack_duration;
    this->cool_down_duration = cool_down_duration;
    attack_sound = Mix_LoadWAV(attack_soundd);
}



void GameObject::meleeattack()
{
    Uint32 ct = SDL_GetTicks();

    if(isattacking && isreadyforattack)
    {
        lastattacktime = ct;
    }
    if(ct - lastattacktime <= shortrange_weapon.cool_down_duration) isreadyforattack = false;
    else isreadyforattack = true;
    if(lastattacktime == 0)
    {
        isreadyforattack = true;
    }
    if(object_type == 1 && isdead) isreadyforattack = false;
    if(ct - lastattacktime > shortrange_weapon.attack_duration) isattacking = false;
}

void GameObject::calculate_melee_hitbox()
{
    double angle_rad = objectangle * M_PI / 180.0;
    shortrange_weapon.hitbox.x = (double)(dstRect.x + dstRect.w/2 + shortrange_weapon.range * cos(angle_rad)) - shortrange_weapon.hitbox.w/2;
    shortrange_weapon.hitbox.y = (double)(dstRect.y + dstRect.h/2 + shortrange_weapon.range * sin(angle_rad)) - shortrange_weapon.hitbox.h/2;
}

//Constructor
GameObject::GameObject(SDL_Renderer* renderer,const char* texturePath,const char* d_sound,int x,int y,int w,int h,int objectspeed,int objecthp,double objectangle,int numofframe,Uint16 walking_duration,int object_type) {
    texture = IMG_LoadTexture(renderer,texturePath);
    dead_sound = Mix_LoadWAV(d_sound);
    complex_texture a;
    a.texture = texture;
    a.numofframe = numofframe;
    textures.push_back(a);
    srcRect = {0, 0, w, h};
    dstRect = {x, y, w, h};
    this->objectspeed = objectspeed;
    this->objecthp = objecthp;
    objectspeed2 = objectspeed;
    this->objectangle = objectangle;
    this->numofframe = numofframe;
    this->walking_duration = walking_duration;
    this->object_type = object_type;
}
//animation + texture;
GameObject::~GameObject() {
    // Free all textures in the vector
    for (auto& tex : textures) {
        if (tex.texture) {
            SDL_DestroyTexture(tex.texture);
        }
    }

    // Then free the current texture if it's not in the vector
    if (texture) {
        SDL_DestroyTexture(texture);
    }

}
void GameObject::setSourceRect(int x, int y, int w, int h) {
    srcRect = {x, y, w, h};
}
void GameObject::setDestinationRect(int x, int y, int w, int h) {
    dstRect = {x, y, w, h};
}

void GameObject::anothertexture()
{
    texture = textures[texture_index].texture;
    numofframe = textures[texture_index].numofframe;
    currentframe = 0;
}

void GameObject::emplace_backtexture(SDL_Renderer* renderer, const char* texturePath, int numofframe) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, texturePath);
    textures.emplace_back(complex_texture{newTexture, numofframe});
}

void GameObject::move(int dx, int dy) {
    dstRect.x += dx;
    dstRect.y += dy;
}

void GameObject::check_if_died()
{
    if(objecthp <= 0) isdead = true;
}

void GameObject::betterrender(double angle,SDL_Renderer* renderer)
{
    SDL_RenderCopyEx(renderer, texture, &srcRect, &dstRect,angle,nullptr,SDL_FLIP_NONE);
}


bool checkcollision(const SDL_Rect& A,const SDL_Rect& B){
    if(A.x + A.w <= B.x || B.x + B.w <= A.x) return false;
    if(A.y + A.h <= B.y || B.y + B.h <= A.y) return false;
    return true;
}
void follow(GameObject& follower,GameObject& target)
{
    int dx = target.dstRect.x - follower.dstRect.x;
    int dy = target.dstRect.y - follower.dstRect.y;
    double distance = sqrt(dx * dx + dy * dy);

    if (distance > 0) {
        follower.dstRect.x += static_cast<int>(follower.objectspeed * dx / distance);
        follower.dstRect.y += static_cast<int>(follower.objectspeed * dy / distance);
    }
}

int clamp(int value, int min_value, int max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

Uint32 getPixel(SDL_Surface* surface, int x, int y) {
    Uint8* pixelData = (Uint8*)surface->pixels;
    int bytesPerPixel = surface->format->BytesPerPixel;
    return *(Uint32*)(pixelData + y * surface->pitch + x * bytesPerPixel);
}

bool checkPerPixelCollision(SDL_Surface* spriteA, SDL_Rect rectA,SDL_Surface* spriteB, SDL_Rect rectB)
{
    int xStart = std::max(rectA.x, rectB.x);
    int xEnd   = std::min(rectA.x + rectA.w, rectB.x + rectB.w);
    int yStart = std::max(rectA.y, rectB.y);
    int yEnd   = std::min(rectA.y + rectA.h, rectB.y + rectB.h);

    for (int y = yStart; y < yEnd; y++) {
        for (int x = xStart; x < xEnd; x++) {
            int ax = x - rectA.x, ay = y - rectA.y;
            int bx = x - rectB.x, by = y - rectB.y;

            Uint32 pixelA = getPixel(spriteA, ax, ay);
            Uint32 pixelB = getPixel(spriteB, bx, by);

            if (pixelA != 0x00000000 && pixelB != 0x00000000) {
                return true;
            }
        }
    }
    return false;
}

void updateCamera(SDL_Rect &camera, GameObject& player, int SCREEN_WIDTH, int SCREEN_HEIGHT, int WORLD_WIDTH, int WORLD_HEIGHT)
{
    camera.x = player.dstRect.x + player.dstRect.w / 2 - SCREEN_WIDTH / 2;
    camera.y = player.dstRect.y + player.dstRect.h / 2 - SCREEN_HEIGHT / 2;

    // Giữ camera trong giới hạn của thế giới
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > WORLD_WIDTH - SCREEN_WIDTH) camera.x = WORLD_WIDTH - SCREEN_WIDTH;
    if (camera.y > WORLD_HEIGHT - SCREEN_HEIGHT) camera.y = WORLD_HEIGHT - SCREEN_HEIGHT;
}

void rendercopytocamera(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects)
{
    for (std::size_t i = 0; i < gameobjects.size(); i++)
    {
        SDL_Rect CameraRect = {
            gameobjects[i]->dstRect.x - camera.x,
            gameobjects[i]->dstRect.y - camera.y,
            gameobjects[i]->dstRect.w,
            gameobjects[i]->dstRect.h
        };

        SDL_RenderCopyEx(renderer, gameobjects[i]->texture, &gameobjects[i]->srcRect, &CameraRect,gameobjects[i]->objectangle,nullptr,SDL_FLIP_NONE);
    }
}

void rendercopytocamera_for_alive(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects)
{
    for (std::size_t i = 0; i < gameobjects.size(); i++)
    {
        if((gameobjects[i]->isdead)) continue;
        SDL_Rect CameraRect = {
            gameobjects[i]->dstRect.x - camera.x,
            gameobjects[i]->dstRect.y - camera.y,
            gameobjects[i]->dstRect.w,
            gameobjects[i]->dstRect.h
        };

        SDL_RenderCopyEx(renderer, gameobjects[i]->texture, &gameobjects[i]->srcRect, &CameraRect,gameobjects[i]->objectangle,nullptr,SDL_FLIP_NONE);
    }
}

void rendercopytocamera_for_dead(SDL_Renderer* renderer, SDL_Rect camera, std::vector<GameObject*>& gameobjects)
{
    for (std::size_t i = 0; i < gameobjects.size(); i++)
    {
        if(!(gameobjects[i]->isdead)) continue;
        SDL_Rect CameraRect = {
            gameobjects[i]->dstRect.x - camera.x,
            gameobjects[i]->dstRect.y - camera.y,
            gameobjects[i]->dstRect.w,
            gameobjects[i]->dstRect.h
        };

        SDL_RenderCopyEx(renderer, gameobjects[i]->texture, &gameobjects[i]->srcRect, &CameraRect,gameobjects[i]->objectangle,nullptr,SDL_FLIP_NONE);
    }
}

void rendercopytocamera_for_bullet(SDL_Renderer* renderer, SDL_Rect camera, std::vector<bullet*>& bullets)
{
    for(auto z : bullets)
    {
        SDL_Rect camera_rect = {z->dstRect.x - camera.x,z->dstRect.y - camera.y,z->dstRect.w,z->dstRect.h};
        SDL_RenderCopyEx(renderer,z->texture,&z->srcRect,&camera_rect,z->bullet_angle,nullptr,SDL_FLIP_NONE);
    }
}

void Initializer(SDL_Window* &window,SDL_Renderer* &renderer,int SCREEN_WIDTH,int SCREEN_HEIGHT)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow("Murderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}
void handleMovement(GameObject& object, int speed) {
    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_W])
    {
        object.dstRect.y -= speed; // Đi lên
    }
    if (keystates[SDL_SCANCODE_S])
    {
        object.dstRect.y += speed;
    } // Đi xuống
    if (keystates[SDL_SCANCODE_A])
    {
        object.dstRect.x -= speed;
    } // Đi trái
    if (keystates[SDL_SCANCODE_D])
    {
        object.dstRect.x += speed;
    } // Đi phải
}

void EndEverything(SDL_Window* &window,SDL_Renderer* &renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void greater_check_if_died(std::vector<GameObject*> &objects,int map_x,int map_y)
{
    for(auto it = objects.begin();it != objects.end();)
    {
        if((*it)->dstRect.x < -50 || (*it)->dstRect.x > (map_x) || (*it)->dstRect.y < -50 || (*it)->dstRect.y > (map_y))
        {
            (*it)->objecthp -= 100;
        }
        (*it)->check_if_died();
        if((*it)->object_type == 1 && (*it)->isdead) (*it)->go_through_able = true;
        it++;
    }
}

void ObjectKiller(std::vector<GameObject*> &objects) {
    for(auto it = objects.begin(); it != objects.end();)
        {
            if((*it)->isdead && (*it)->played_dead_sound)
                { delete *it; it = objects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

void angle_otom(GameObject &player,const SDL_Rect &camera)
{
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        float worldMouseX = mouseX + camera.x;
        float worldMouseY = mouseY + camera.y;

        float dx = worldMouseX - (player.dstRect.x + player.dstRect.w / 2);
        float dy = worldMouseY - (player.dstRect.y + player.dstRect.h / 2);
        player.objectangle = atan2(dy, dx) * 180.0 / M_PI;
}

void angle_to_player(SDL_Rect& tempdstRect,GameObject& npc,GameObject& player)
{
    float dx = npc.dstRect.x - tempdstRect.x;
    float dy = npc.dstRect.y - tempdstRect.y;
    angle_a_to_b(npc,player);
    if(dx != 0 || dy != 0)npc.isrunning = true;
    else npc.isrunning = false;
}

void angle_otomovement(SDL_Rect& tempdstRect,GameObject& npc)
{
    float dx = npc.dstRect.x - tempdstRect.x;
    float dy = npc.dstRect.y - tempdstRect.y;
    if(dx != 0 || dy != 0)
    {
        npc.objectangle = atan2(dy,dx)*180/ M_PI;
        npc.isrunning = true;
    }
    else npc.isrunning = false;
}

void angle_a_to_b(GameObject &a,GameObject &b)
{
    float dx = b.dstRect.x - a.dstRect.x;
    float dy = b.dstRect.y - a.dstRect.y;
    a.objectangle = atan2(dy, dx) * 180.0 / M_PI;
}

void random_movement(GameObject& a)
{
    int method1 = rand() % 101;
    if(method1 >= 90) a.objectangle = rand() % 361 - 180;
    int method = rand() % 11;
    if(method <= 7)
    {
    a.dstRect.x += a.objectspeed * cos(a.objectangle*M_PI/180.0);
    a.dstRect.y += a.objectspeed * sin(a.objectangle*M_PI/180.0);
    }
}

void check_if_moved(GameObject& moving_things, SDL_Rect& temprect) {
    moving_things.isrunning = (moving_things.dstRect.x != temprect.x || moving_things.dstRect.y != temprect.y);
}

bool check_if_collided(const GameObject& moving_things,const std::vector<GameObject*>& Other_object)
{
    for(auto x : Other_object)
    {
        if(checkcollision(x->dstRect,moving_things.dstRect) && !(x->go_through_able)) return true;
    }
    return false;
}

bool check_if_collided_plus_physic_1(GameObject& moving_things,const std::vector<GameObject*>& Other_object)
{
    bool result = false;
    for(auto x : Other_object)
    {
        int tmp_speed = moving_things.objectspeed;
        while(checkcollision(x->dstRect,moving_things.dstRect) && !(x->go_through_able))
        {
            moving_things.objectspeed = 0;
            physics_1(moving_things,*x);
            result = true;
        }
        moving_things.objectspeed = tmp_speed;
    }
    return result;
}

void check_attack_and_calculate_damage(const GameObject& attacker,std::vector<GameObject*>& Other_object)
{
    for(auto& x : Other_object)
    {
        if(attacker.isattacking && checkcollision(attacker.shortrange_weapon.hitbox,x->dstRect))
        {
            x->objecthp -= attacker.shortrange_weapon.damage;
        }
    }
}

void check_attack_and_calculate_damage_for_a_and_b(const GameObject& attacker,GameObject& victim)
{
    if(attacker.isattacking && checkcollision(attacker.shortrange_weapon.hitbox,victim.dstRect))
       {
           if(attacker.objecthp > 0 || attacker.object_type == 1)victim.objecthp -= attacker.shortrange_weapon.damage;
       }
}

int calculate_distance(GameObject& a,GameObject& b)
{
    int x = abs(a.dstRect.x - b.dstRect.x);
    int y = abs(a.dstRect.y - b.dstRect.y);
    return sqrt(x*x + y*y);
}

void the_ultimate_status_handler_for_things(std::vector<GameObject*>other_objects,std::vector<GameObject*>npcs,GameObject& player)
{
    for(auto x : other_objects)
    {
        if(x->isdead) x->isattacking = true;
        x->meleeattack();
        x->calculate_melee_hitbox();
        check_attack_and_calculate_damage_for_a_and_b(*x,player);
        check_attack_and_calculate_damage(*x,npcs);
    }
}

void physics_1(GameObject& a, GameObject& b) {
    float dx = a.dstRect.x+a.dstRect.w/2 - b.dstRect.x - b.dstRect.w;
    float dy = a.dstRect.y + a.dstRect.h/2 - b.dstRect.y - b.dstRect.h/2;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance == 0) return;

    float pushX = (dx / distance) * 5;
    float pushY = (dy / distance) * 5;

    a.dstRect.x += pushX;
    a.dstRect.y += pushY;
}

void the_ultimate_movement_and_status_handler(SDL_Renderer* renderer,std::vector<GameObject*>npcs,const std::vector<std::vector<cell>>& map_grid,int cellsize,std::vector<GameObject*>other_objects,GameObject& player,std::vector<bullet*>& bullets)
{
        int i = 0;
        std::vector<SDL_Rect> npcstempsrcrect;
        for(auto &x : npcs)
        {
            npcstempsrcrect.push_back(x->dstRect);
            if(x->isdead) x->objectspeed = 0;
        }
        for(auto &x : npcs)
        {
            if(x->isdead)
            {
                continue;
            }
            if(calculate_distance(*x,player) <= 1000)follow_the_vector(*x,map_grid,cellsize);
            else random_movement(*x);
            if(check_if_collided_plus_physic_1(*x,other_objects))x->isattacking = true;

            if((calculate_distance(*x,player) < 150) || (calculate_distance(*x,player) <= 500 && x->shortrange_weapon.weapon_type == 1))
            {
                x->isattacking = true;
            }
            if(x->isattacking && !(x->played_attack_sound) && x->shortrange_weapon.weapon_type == 1)
            {
                double rad = x->objectangle * M_PI / 180.0;
                bullet *bullet1 = new bullet(renderer,5000,40,"bullets/bullet1.png",(x->dstRect.x+x->dstRect.w/2)+40*cos(rad),(x->dstRect.y+x->dstRect.h/2)+40*sin(rad),16,6,x->objectangle);
                bullets.push_back(bullet1);
            }
            x->meleeattack();
            x->calculate_melee_hitbox();
            check_attack_and_calculate_damage_for_a_and_b(*x,player);
            check_attack_and_calculate_damage(*x,other_objects);
        }
        for(auto &x : npcs)
        {
            if(x->isdead)
            {
                i++;
                continue;
            }
            if(calculate_distance(*x,player) <= 1000) angle_to_player(npcstempsrcrect[i++],*x,player);
            else angle_otomovement(npcstempsrcrect[i++],*x);
        }
}

void the_ultimate_movement_and_status_handler_for_player(SDL_Renderer* renderer,GameObject& player,const SDL_Rect& camera,std::vector<GameObject*>other_objects,std::vector<GameObject*> npcs,std::vector<bullet*>& bullets)
{
    if(player.isdead)
    {
        player.objectspeed = 0;
        return;
    }
    SDL_Rect tempplayerdstrect = player.dstRect;
    handleMovement(player,player.objectspeed);
    angle_otom(player,camera);
    player.meleeattack();
    if(player.isattacking)
    {
        player.calculate_melee_hitbox();
        check_attack_and_calculate_damage(player,npcs);
        check_attack_and_calculate_damage(player,other_objects);
        if(player.shortrange_weapon.weapon_type == 1 && !player.played_attack_sound)
        {
                double rad = player.objectangle * M_PI / 180.0;
                bullet *bullet1 = new bullet(renderer,5000,30,"bullets/bullet1.png",(player.dstRect.x+player.dstRect.w/2)+40*cos(rad),(player.dstRect.y+player.dstRect.h/2)+40*sin(rad),16,6,player.objectangle);
                bullets.push_back(bullet1);
        }
    }
    if(check_if_collided(player,other_objects)) player.dstRect = tempplayerdstrect;

    check_if_moved(player,tempplayerdstrect);
}

void the_ultimate_animation_handler(GameObject& moving_things, int frame_duration_for_walking_texture)
{
    if(moving_things.object_type == 1)
    {
        int new_texture_index = moving_things.isattacking ? 1 : 0;
        if(moving_things.texture_index != new_texture_index)
        {
            moving_things.texture_index = new_texture_index;
            moving_things.anothertexture();
        }
        if(moving_things.isdead)
        {
            if(!moving_things.isattacking)
            {
            moving_things.texture_index = 2;
            moving_things.anothertexture();
            moving_things.setSourceRect(0,0,1000,1000);
            return;
            }
        }
        if(moving_things.isattacking)
        {
            int width, height;
            SDL_QueryTexture(moving_things.texture, nullptr, nullptr, &width, &height);
            int frame_width = width / moving_things.numofframe;
            Uint32 ct = SDL_GetTicks();
            int frame_duration_for_attacking = moving_things.shortrange_weapon.attack_duration / moving_things.numofframe;
            if (ct - moving_things.lastattacktime >= frame_duration_for_attacking * (moving_things.currentframe + 1))
            {
            moving_things.currentframe = (moving_things.currentframe + 1) % moving_things.numofframe;
            moving_things.setSourceRect(frame_width * moving_things.currentframe, 0, frame_width, height);
            }
            return;
        }
    }

    if(moving_things.object_type == 0)
    {
    if(moving_things.isdead)
    {
        moving_things.texture_index = 2;
        moving_things.anothertexture();
        moving_things.setSourceRect(0,0,1000,1000);
        moving_things.setDestinationRect(moving_things.dstRect.x,moving_things.dstRect.y,120,110);
        return;
    }
    int new_texture_index = moving_things.isattacking ? 1 : 0;
    if (moving_things.texture_index != new_texture_index) {
        moving_things.texture_index = new_texture_index;
        moving_things.anothertexture();
    }

    int width, height;
    SDL_QueryTexture(moving_things.texture, nullptr, nullptr, &width, &height);
    int frame_width = width / moving_things.numofframe;
    Uint32 ct = SDL_GetTicks();
    int frame_duration_for_attacking = moving_things.shortrange_weapon.attack_duration / moving_things.numofframe;

    if (moving_things.isattacking) {
        if (ct - moving_things.lastattacktime >= frame_duration_for_attacking * (moving_things.currentframe + 1)) {
            moving_things.currentframe = (moving_things.currentframe + 1) % moving_things.numofframe;
            moving_things.setSourceRect(frame_width * moving_things.currentframe, 0, frame_width, height);
        }
        return;
    }


    if (moving_things.isrunning) {
        if (ct - moving_things.lastframetime >= frame_duration_for_walking_texture) {
            moving_things.currentframe = (moving_things.currentframe + 1) % moving_things.numofframe;
            moving_things.setSourceRect(frame_width * moving_things.currentframe, 0, frame_width, height);
            moving_things.lastframetime = ct;
        }
    }
    else { // Nếu đứng yên
        moving_things.setSourceRect(0, 0, frame_width, height);
        moving_things.currentframe = 0;
    }
    }
}



void the_more_ultimate_animation_handler(std::vector<GameObject*>moving_things)
{
    for(auto it = moving_things.begin(); it != moving_things.end(); it++)
    {
            the_ultimate_animation_handler(*(*it),(*it)->walking_duration);
    }
}

void the_ultimate_sound_effects_handler(std::vector<GameObject*>moving_things)
{
     for (auto it : moving_things)
    {
        if(it->isdead && !it->played_dead_sound)
        {
            Mix_PlayChannel(-1, it->dead_sound, 0);
            it->played_dead_sound = true;
        }
        if(it->object_type == 0)
        {
            if(it->isreadyforattack) it->played_attack_sound = false;
            if(it->isattacking && !it->played_attack_sound)
            {
            Mix_PlayChannel(-1,(*it).shortrange_weapon.attack_sound,0);
            it->played_attack_sound = true;
            }
        }
    }
}

void the_ultimate_sound_effects_handler_for_single_object(GameObject& moving_thing)
{
    if(moving_thing.isdead && !moving_thing.played_dead_sound)
    {
        Mix_PlayChannel(-1,moving_thing.dead_sound,0);
        moving_thing.played_dead_sound = true;
    }
    if(moving_thing.isreadyforattack) moving_thing.played_attack_sound = false;
    if(moving_thing.isattacking && !moving_thing.played_attack_sound)
    {
        Mix_PlayChannel(-1,moving_thing.shortrange_weapon.attack_sound,0);
        moving_thing.played_attack_sound = true;
    }
}


void random_npcs_generator(SDL_Renderer* renderer, std::vector<GameObject*>& moving_things,const std::vector<std::vector<cell>>& map_grid,int mapsize_x,int mapsize_y,int cell_size)
{
    int method = rand() % 3;

    int x,y;
    do
    {
        x = rand() % (mapsize_y+1);
        y = rand() % (mapsize_x+1);
    }while(map_grid[y/cell_size][x/cell_size].cost < 10 || map_grid[y/cell_size][x/cell_size].cost == 1000);

    if(method == 0)
    {

        GameObject *npc1 = new GameObject(renderer,"npcs/npc1.png","sounds/dead.wav",x,y,74,74,10,300,0,8,20,0);
        npc1->shortrange_weapon = {70,70,1000,30,320,500,"sounds/metal_pipe.wav",0};
        npc1->emplace_backtexture(renderer,"npcs/npc1-a.png",7);
        npc1->emplace_backtexture(renderer,"npcs/npc1-d.png",1);
        npc1->setSourceRect(0,0,32,32);
        moving_things.push_back(npc1);
    }
    if(method == 1)
    {
        GameObject *npc2 = new GameObject(renderer,"npcs/npc2.png","sounds/dead.wav",x,y,80,80,6,300,0,8,16,0);
        npc2->shortrange_weapon = {100,100,0,0,100,600,"sounds/gun_shot.wav",1};
        npc2->emplace_backtexture(renderer,"npcs/npc2-a.png",3);
        npc2->emplace_backtexture(renderer,"npcs/npc2-d.png",1);
        npc2->setSourceRect(0,0,32,32);
        moving_things.push_back(npc2);
    }
    if(method == 2)
    {
        GameObject *npc3 = new GameObject(renderer,"npcs/npc3.png","sounds/dead.wav",x,y,93,93,8,300,0,8,16,0);
        npc3->shortrange_weapon = {70,70,1100,30,300,850,"sounds/metal_pipe.wav",0};
        npc3->emplace_backtexture(renderer,"npcs/npc3-a.png",7);
        npc3->emplace_backtexture(renderer,"npcs/npc3-d.png",1);
        npc3->setSourceRect(0,0,44,44);
        moving_things.push_back(npc3);
    }
}

int count_dead_things(const std::vector<GameObject*>& moving_things)
{
    int sum = 0;
    for(auto x : moving_things)
    {
        if(x->isdead) sum++;
    }
    return sum;
}

void handle_movement_of_bullet(std::vector<bullet*>& bullets)
{
    for(auto z : bullets)
    {
        double rad = z->bullet_angle * M_PI / 180.0;
        z->dstRect.x += cos(rad) * z->speed;
        z->dstRect.y += sin(rad) * z->speed;
    }
}

void handle_collision_of_bullet(std::vector<bullet*>& bullets,std::vector<GameObject*> things)
{
    for(auto z : bullets)
    {
        for(auto m : things)
        {
            if(checkcollision(z->dstRect,m->dstRect))
            {
                if(m->isdead) continue;
                m->objecthp -= z->damage;
                z->isdead = true;
            }
        }
    }
}

void handle_status_and_delete_dead_bullets(std::vector<bullet*>& bullets, int map_x, int map_y)
{
    for (auto it = bullets.begin(); it != bullets.end(); )
    {
        if ((*it)->dstRect.x < 0 || (*it)->dstRect.x > (map_x + 100) || (*it)->dstRect.y < 0 || (*it)->dstRect.y > (map_y + 100) || (*it)->isdead)
        {
            SDL_DestroyTexture((*it)->texture);
            memset(*it, 0, sizeof(**it));
            delete (*it);
            it = bullets.erase(it);
        }
        else
        {
            ++it;
        }
    }
    bullets.shrink_to_fit();  // Giải phóng bộ nhớ dư thừa
}



//celllllllll










cell::cell()
{
    x = 0;
    y = 0;
    cost = -2;
    flowVector ={0,0};
}

bool isFull(std::vector<std::vector<cell>>& matrix,int default_value){
    int size_x = matrix.size();
    int size_y = matrix[0].size();
    if (matrix.empty() || matrix[0].empty()) return false;
    for(int x = 0;x < size_x;x++)
    {
        for(int y = 0;y < size_y;y++)
        {
            if(matrix[x][y].cost == default_value)
            {
                return false;
            }
        }
    }
    return true;
}

void computeCostField(std::vector<std::vector<cell>>& grid) {
    if (grid.empty() || grid[0].empty()) return;

    int size_x = grid.size();
    int size_y = grid[0].size();

    while (!isFull(grid, -2)) {
        for (int x = 0; x < size_x; x++) {
            for (int y = 0; y < size_y; y++) {
                if (grid[x][y].cost != 1000 && grid[x][y].cost != -2) {
                    int new_cost = grid[x][y].cost + 1;

                    if (x + 1 < size_x && grid[x + 1][y].cost == -2)
                        grid[x + 1][y].cost = new_cost;

                    if (x - 1 >= 0 && grid[x - 1][y].cost == -2)
                        grid[x - 1][y].cost = new_cost;

                    if (y + 1 < size_y && grid[x][y + 1].cost == -2)
                        grid[x][y + 1].cost = new_cost;

                    if (y - 1 >= 0 && grid[x][y - 1].cost == -2)
                        grid[x][y - 1].cost = new_cost;
                }
            }
        }
    }
}




void drawVector(std::vector<std::vector<cell>>& arr) {
    int sizex = arr.size();
    int sizey = arr[0].size();
    for (int x = 0; x < sizex; x++) {
        for (int y = 0; y < sizey; y++) {

            int minCost = arr[x][y].cost;
            SDL_Point bestDir = {0, 0};

            if (x + 1 < sizex && arr[x + 1][y].cost < minCost) {
                minCost = arr[x + 1][y].cost;
                bestDir = {1, 0};
            }
            if (x - 1 >= 0 && arr[x - 1][y].cost < minCost) {
                minCost = arr[x - 1][y].cost;
                bestDir = {-1, 0};
            }
            if (y + 1 < sizey && arr[x][y + 1].cost < minCost) {
                minCost = arr[x][y + 1].cost;
                bestDir = {0, 1};
            }
            if (y - 1 >= 0 && arr[x][y - 1].cost < minCost) {
                minCost = arr[x][y - 1].cost;
                bestDir = {0, -1};
            }

            arr[x][y].flowVector = bestDir;
        }
    }
}

void initialcostfield(std::vector<std::vector<cell>>& grid,const std::vector<GameObject*>other_objects,int cellsize)
{
      for(auto x : other_objects)
      {
          int widthcell = ceil((double)x->dstRect.w / (double)cellsize);
          int heightcell = ceil((double)x->dstRect.h / (double)cellsize);
          int xcell = x->dstRect.x / cellsize;
          int ycell = x->dstRect.y/cellsize;
          for(int i = xcell;i < xcell+widthcell;i++)
          {
              for(int y = ycell;y < ycell+heightcell;y++)
              {
                  grid[y][i].cost = 1000;
              }
          }
      }
}

bool isValid(int x, int y, int sizex, int sizey, const std::vector<std::vector<cell>>& arr) {
    return x >= 0 && x < sizex && y >= 0 && y < sizey && arr[x][y].cost != -1;
}

void drawbetterVector(std::vector<std::vector<cell>>& arr) {
    if (arr.empty() || arr[0].empty()) return;

    int sizex = arr.size();
    int sizey = arr[0].size();

    const std::vector<SDL_Point> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, // 4 hướng chính
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1} // 4 hướng chéo
    };

    for (int x = 0; x < sizex; x++) {
        for (int y = 0; y < sizey; y++) {

            int minCost = arr[x][y].cost;
            SDL_Point bestDir = {0, 0};

            for (const auto& dir : directions) {
                int nx = x + dir.x, ny = y + dir.y;
                if (isValid(nx, ny, sizex, sizey, arr) && arr[nx][ny].cost < minCost) {
                    minCost = arr[nx][ny].cost;
                    bestDir = dir;
                }
            }

            arr[x][y].flowVector = bestDir;
        }
    }
}

void follow_the_vector(GameObject& npc,const std::vector<std::vector<cell>>& grid, int cellsize) {
    int gridX = npc.dstRect.y / cellsize;
    int gridY = npc.dstRect.x / cellsize;

    if (gridX < 0 || gridX >= grid.size() || gridY < 0 || gridY >= grid[0].size()) return;

    SDL_Point direction = grid[gridX][gridY].flowVector;

    npc.dstRect.x = npc.dstRect.x + direction.y * npc.objectspeed;
    npc.dstRect.y = npc.dstRect.y + direction.x * npc.objectspeed;
}

bool checkClick(SDL_Rect button, int x, int y) {
    return (x > button.x && x < button.x + button.w &&
            y > button.y && y < button.y + button.h);
}

void game()
{
    srand(time(0));
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    Initializer(window,renderer,1280,720);
    Mix_AllocateChannels(50);

    SDL_Rect startButton = {100, 100, 300, 50};
    SDL_Rect quitButton = {100, 300, 300, 50};

    SDL_Texture* start = IMG_LoadTexture(renderer,"menu/start.png");
    SDL_Texture* background1 = IMG_LoadTexture(renderer,"menu/background.png");
    SDL_Texture* quit = IMG_LoadTexture(renderer,"menu/quit.png");

    Mix_Music* background_music = Mix_LoadMUS("background.mp3");
    Mix_PlayMusic(background_music, -1);





    SDL_Event event;
    bool running1 = true;
    bool running = true;
    while(running1)
    {
        SDL_RenderCopy(renderer,background1,nullptr,nullptr);
        SDL_RenderCopy(renderer,start,nullptr,&startButton);
        SDL_RenderCopy(renderer,quit,nullptr,&quitButton);
        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
            {
                running1 = false;
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x = event.button.x, y = event.button.y;
                if (checkClick(startButton, x, y))
                {
                SDL_Rect camera = {0,0,1280,720};
                GameObject player(renderer,"player/player1.png","sounds/dead-p.wav",0,0,70,70,11,10000,0,8,18,0);
                player.shortrange_weapon = {75,75,300,30,320,420,"sounds/knife1.wav",0};
                player.emplace_backtexture(renderer,"player/player2.png",8);
                player.emplace_backtexture(renderer,"player/player3.png",1);



                GameObject bosscar(renderer,"things/bosscar.png","sounds/explosion.wav",0,0,103,53,0,30000,0,1,20,1);
                bosscar.setDestinationRect(100,100,200,100);
                bosscar.shortrange_weapon = {500,500,10000,0,400,400,"sounds/explosion.wav",0};
                bosscar.emplace_backtexture(renderer,"things/bosscar-a.png",7);
                bosscar.emplace_backtexture(renderer,"things/bosscar-d.png",1);

                GameObject bosscar2(renderer,"things/bosscar.png","sounds/explosion.wav",0,0,108,53,0,30000,0,1,20,1);
                bosscar2.setDestinationRect(100,300,200,100);
                bosscar2.shortrange_weapon = {500,500,10000,0,400,400,"sounds/explosion.wav",0};
                bosscar2.emplace_backtexture(renderer,"things/bosscar-a.png",7);
                bosscar2.emplace_backtexture(renderer,"things/bosscar-d.png",1);

                GameObject bosscar3(renderer,"things/bosscar.png","sounds/explosion.wav",0,0,108,53,0,30000,0,1,20,1);
                bosscar3.setDestinationRect(100,500,200,100);
                bosscar3.shortrange_weapon = {500,500,10000,0,400,400,"sounds/explosion.wav",0};
                bosscar3.emplace_backtexture(renderer,"things/bosscar-a.png",7);
                bosscar3.emplace_backtexture(renderer,"things/bosscar-d.png",1);

                GameObject wall_1(renderer,"things/wall1.png","sounds/explosion.wav",0,0,50,850,0,100000,0,1,20,1);
                wall_1.setDestinationRect(400,0,50,850);
                wall_1.shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_1.emplace_backtexture(renderer,"things/wall1-a.png",1);
                wall_1.emplace_backtexture(renderer,"things/wall1-d.png",1);

                GameObject wall_2(renderer,"things/wall1.png","sounds/explosion.wav",0,0,50,850,0,100000,0,1,20,1);
                wall_2.setDestinationRect(400,1150,50,850);
                wall_2.shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_2.emplace_backtexture(renderer,"things/wall1-a.png",1);
                wall_2.emplace_backtexture(renderer,"things/wall1-d.png",1);

                GameObject wall_3(renderer,"things/wall2.png","sounds/explosion.wav",0,0,850,50,0,100000,0,1,20,1);
                wall_3.setDestinationRect(450,0,1550,50);
                wall_3.shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_3.emplace_backtexture(renderer,"things/wall2-a.png",1);
                wall_3.emplace_backtexture(renderer,"things/wall2-d.png",1);

                GameObject wall_4(renderer,"things/wall2.png","sounds/explosion.wav",0,0,850,50,0,100000,0,1,20,1);
                wall_4.setDestinationRect(450,1950,1550,50);
                wall_4.shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_4.emplace_backtexture(renderer,"things/wall2-a.png",1);
                wall_4.emplace_backtexture(renderer,"things/wall2-d.png",1);

                GameObject wall_5(renderer,"things/wall1.png","sounds/explosion.wav",0,0,50,850,0,100000,0,1,20,1);
                wall_5.setDestinationRect(1950,50,50,1900);
                wall_5.shortrange_weapon = {400,400,10,0,400,400,"sounds/explosion.wav",0};
                wall_5.emplace_backtexture(renderer,"things/wall1-a.png",1);
                wall_5.emplace_backtexture(renderer,"things/wall1-d.png",1);



                player.setSourceRect(0,0,32,32);
                std::vector<GameObject*> players = {&player};
                std::vector<GameObject*> npcs;
                std::vector<GameObject*> things = {&bosscar,&bosscar2,&bosscar3,&wall_1,&wall_2,&wall_3,&wall_4,&wall_5};
                std::vector<bullet*> bullets;

                SDL_Surface* tempSurface = IMG_Load("maps/map.png");
                SDL_Texture* background = SDL_CreateTextureFromSurface(renderer, tempSurface);
                SDL_FreeSurface(tempSurface);
                int bgWidth, bgHeight;
                SDL_QueryTexture(background, NULL, NULL, &bgWidth, &bgHeight);
                running = true;
                    while(running)
                    {
                    while(SDL_PollEvent(&event))
                    {
                    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE || event.type == SDL_QUIT || player.isdead)
                    {
                    running = false;
                    }
                    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                    {
                    player.isattacking = true;
                    }
                    }

                    if(player.objecthp < 9990) player.objecthp += 10;

                    SDL_RenderClear(renderer);

                    SDL_RenderCopy(renderer,background,&camera,NULL);

                    std::vector<std::vector<cell>> mapgrid(40,std::vector<cell>(40));
                    initialcostfield(mapgrid,things,50);
                    mapgrid[clamp(player.dstRect.y/50,0,39)][clamp(player.dstRect.x/50,0,39)].cost = 0;
                    computeCostField(mapgrid);
                    drawbetterVector(mapgrid);


                    if(npcs.size() < 10) random_npcs_generator(renderer,npcs,mapgrid,2000,2000,50);

                    the_ultimate_movement_and_status_handler_for_player(renderer,player,camera,things,npcs,bullets);
                    greater_check_if_died(npcs,2000,2000);
                    the_ultimate_movement_and_status_handler(renderer,npcs,mapgrid,50,things,player,bullets);
                    the_ultimate_status_handler_for_things(things,npcs,player);

                    handle_movement_of_bullet(bullets);
                    handle_status_and_delete_dead_bullets(bullets,2000,2000);
                    handle_collision_of_bullet(bullets,npcs);
                    handle_collision_of_bullet(bullets,things);
                    handle_collision_of_bullet(bullets,players);

                    the_ultimate_animation_handler(player,20);
                    the_more_ultimate_animation_handler(npcs);
                    the_more_ultimate_animation_handler(things);


                    the_ultimate_sound_effects_handler_for_single_object(player);
                    the_ultimate_sound_effects_handler(npcs);
                    the_ultimate_sound_effects_handler(things);



                    rendercopytocamera(renderer,camera,things);
                    rendercopytocamera_for_dead(renderer,camera,players);
                    rendercopytocamera_for_dead(renderer,camera,npcs);
                    rendercopytocamera_for_bullet(renderer,camera,bullets);
                    rendercopytocamera_for_alive(renderer,camera,players);
                    rendercopytocamera_for_alive(renderer,camera,npcs);
                    updateCamera(camera,player,1280,720,bgWidth,bgHeight);

                    SDL_SetRenderDrawColor(renderer,255,0,0,255);

                    SDL_RenderPresent(renderer);








                    greater_check_if_died(players,2000,2000);
                    greater_check_if_died(things,2000,2000);
                    if(count_dead_things(npcs) >= 6) ObjectKiller(npcs);
                    SDL_Delay(16);
                    }
                    greater_check_if_died(npcs,2000,2000);
                    ObjectKiller(npcs);

                }

                if (checkClick(quitButton, x, y)) running1 = false;
            }
        }
    }
    EndEverything(window,renderer);

}





