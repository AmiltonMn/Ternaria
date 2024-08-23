#include "imports.h"

HWND Ghwnd;
HINSTANCE GInstance;


// Desenha retângulo
void DrawRect(HDC hdc, const RECT * Rect, COLORREF Color)
{
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, Color);
    Rectangle(hdc, Rect->left, Rect->top, Rect->right, Rect->bottom);
}

// Colisão geral
int Collision(const RECT * R1, const RECT * R2)
{
    RECT Inter;
    if(IntersectRect(&Inter, R1, R2))
    {
        if(Inter.right - Inter.left > Inter.bottom - Inter.top)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }
    return 0;
}

// Função de colisão de blocos
block * MapCollision(const RECT * R)
{
    for(int i = 0; i < Map.Size; ++i)
    {
        block * B = (block *) Map.List[i];
        if(Collision(&B->hitbox, R))
        {
            return B;
        }
    }
    return NULL;
}

// Função de destruir blocos
int DestroyBlocks(character * Player, POINT Mouse)
{
    if(sqrt(pow(abs(Mouse.x - ((Player->hitbox.left + Player->hitbox.right) / 2)), 2) + pow(abs(Mouse.y - ((Player->hitbox.bottom + Player->hitbox.top) / 2)), 2)) > 128)
    {
        return 0;
    }
    for(int i = 0; i < Map.Size; ++i)
    {
        block * B = (block *) Map.List[i];
        if(PtInRect(&B->hitbox, Mouse))
        {
            if (B->life == 0)
            {
                free(B);
                DArrayRemove(&Map, i);
                // WriteArchive(&Map);
                return 1;
            }
            B->life -= 1;
        }
    }
    return 0;
}

// Função de colocar blocos, caso não tenha nenhum bloco adjacente a outro, não é possível colocar bloco
int PlaceBlocks(character * Player, zombie * Zombie, POINT Mouse, int Type)
{
    if(sqrt(pow(abs(Mouse.x - ((Player->hitbox.left + Player->hitbox.right) / 2)), 2) + pow(abs(Mouse.y - ((Player->hitbox.bottom + Player->hitbox.top) / 2)), 2)) > 128)
    {
        return 0;
    }
    block * B = malloc(sizeof(block));
    B->x = (Mouse.x + mapax) / 32;
    B->y = (1010 - Mouse.y + mapay) / 32 ;
    B->type = Type;

    switch (B->type)
    {
    case 1:
        B->life = GrassLife;
        break;

    case 2:
        B->life = DirtLife;
        break;

    case 3:
        B->life = LogLife;
        break;

    case 4:
        B->life = LeavesLife;
        break;

    case 5:
        B->life = StoneLife;
        break;  

    default:
        break;
    }

    int canPlace = 0;
    BlockDefine(B);
    if(Collision(&B->hitbox, &Player->hitbox) || Collision(&B->hitbox, &Zombie->hitbox))
    {
        free(B);
        return 0;
    }
    for(int i = 0; i < Map.Size; ++i)
    {
        block * C = (block *) Map.List[i];
        B->hitbox.bottom += 1;
        B->hitbox.top -= 1;
        if(Collision(&B->hitbox, &C->hitbox))
        {
            canPlace = 1;
            if(C->hitbox.bottom > B->hitbox.bottom && C->type == 1)
            {
                C->type = 2;
                BlockDefine(C);
            }  
        }
        B->hitbox.bottom -= 1;
        B->hitbox.top += 1;
        B->hitbox.right += 1;
        B->hitbox.left -= 1;
        if(Collision(&B->hitbox, &C->hitbox))
        {
            canPlace = 1;
        }
        B->hitbox.right -= 1;
        B->hitbox.left += 1;
        if(Collision(&B->hitbox, &C->hitbox))
        {
            canPlace = 0;
            break;
        }
    }
    if(!canPlace)
    {
        free(B);
        return 0;
    }
    DArrayAdd(&Map, B);
    AddBlockArchive(&Map, B);
    return 1;
}

//Função para criar o zumbi
void SpawnZombie(zombie * Zombie, const character * Player)
{
    int PlayerX = Player->hitbox.left >> 5;

    if(Zombie->baseLife < 22)
    {
        //Zumbi Pequeno
        Zombie->hitbox.left = ((PlayerX + 20) % 60) * 32;
        Zombie->hitbox.right = Zombie->hitbox.left + 31;
        Zombie->hitbox.top = 812;
        Zombie->hitbox.bottom = Zombie->hitbox.top + 63;
        while(MapCollision(&Zombie->hitbox))
        {
            Zombie->hitbox.top -= 32;
            Zombie->hitbox.bottom -= 32;
        }
        Zombie->life = Zombie->baseLife;
        Zombie->damage = 2;
        Zombie->canJump = 1;
        Zombie->state = 1;
        Zombie->knockback = 0;
        Zombie->canMove = 1;
        Zombie->baseLife *= 1.5;
    } else {
        //Zumbi Gigante
        Zombie->hitbox.left = ((PlayerX + 20) % 60) * 32;
        Zombie->hitbox.right = Zombie->hitbox.left + 97;
        Zombie->hitbox.top = 512;
        Zombie->hitbox.bottom = Zombie->hitbox.top + 127;
        while(MapCollision(&Zombie->hitbox))
        {
            Zombie->hitbox.top -= 32;
            Zombie->hitbox.bottom -= 32;
        }
        Zombie->damage = 5;
        Zombie->canJump = 1;
        Zombie->state = 1;
        Zombie->canMove = 1;
        Zombie->knockback = 0;
        Zombie->life = 150;
    }

}

//Função para criar o jogador
void SpawnPlayer(character * Player, zombie * Zombie)
{
    CenterPlayer(Player, Zombie);
    if(!MapCollision(&Player->hitbox))
    {
        Player->hitbox.bottom = Player->hitbox.top + 63;
        block * B = malloc(sizeof(block));
        B->type = 1;
        B->x = Player->hitbox.left >> 5;
        B->y = ((977 - Player->hitbox.bottom) >> 5) + 1;
        BlockDefine(B);
        DArrayAdd(&Map, B);
    }
    Player->hitbox.bottom = Player->hitbox.top + 63;
    while(MapCollision(&Player->hitbox))
    {
        Player->hitbox.top -= 32;
        Player->hitbox.bottom -= 32;
    }
    Player->life = 10;
    Player->damage = 1;
    Player->state = 0;
    Player->mainSlot = 0;
    Player->inventory[0].id = 0;
    Player->inventory[1].id = 1;
    Player->inventory[2].id = 2;

    Player->inventory[3].id = 3;
    Player->inventory[3].damage = 2;

    Player->inventory[4].id = 3;
    Player->inventory[4].damage = 3;

    Player->inventory[5].id = 3;
    Player->inventory[5].damage = 5;

    ItemDefine(&Player->inventory[0]);
    ItemDefine(&Player->inventory[1]);
    ItemDefine(&Player->inventory[2]);
    ItemDefine(&Player->inventory[3]);
    ItemDefine(&Player->inventory[4]);
    ItemDefine(&Player->inventory[5]);
    Player->gravity = 0;
    Player->canJump = 1;
    Player->knockback = 0;
    Player->knockbackSide = 0;
    Player->canMove = 1;
}

// Move o zumbi para baixo
void MoveDownZombie(zombie * zombie, int pixels)
{
    zombie->hitbox.top += pixels;
    zombie->hitbox.bottom += pixels;
    block * B = MapCollision(&zombie->hitbox);
    if(B != NULL)
    {
        zombie->gravity = 0;
        zombie->canJump = 1;
        zombie->hitbox.bottom = B->hitbox.top;
        if (zombie->damage < 5)
        {
            zombie->hitbox.top = zombie->hitbox.bottom - 63;
        } else {
            zombie->hitbox.top = zombie->hitbox.bottom - 127;
        }
        zombie->canMove = 1;
    }
}

// Move o zumbi para cima
void MoveUpZombie(zombie * zombie, int pixels)
{
    zombie->hitbox.top -= pixels;
    zombie->hitbox.bottom -= pixels;
    block * B = MapCollision(&zombie->hitbox);
    if(B != NULL)
    {
        zombie->gravity = 0;
        zombie->canJump = 0;
        zombie->hitbox.top = B->hitbox.bottom;
        if(zombie->damage < 5)
        {
            zombie->hitbox.bottom = zombie->hitbox.top + 63;
        } else {
            zombie->hitbox.bottom = zombie->hitbox.top + 127;
        }
    }
}

//Pulo do zumbi
void ZombieJump(zombie * Zombie, int Pixels) 
{
    if(Zombie->canJump == 1)
    {
        if(Zombie->gravity == 15)
        {
            Zombie->canJump = 0;
            Zombie->gravity = 0;
            return;
        }
        MoveUpZombie(Zombie, Pixels);
    }
}

// Move o zumbi para a direita
void MoveRightZombie(zombie * zombie, int pixels)
{
    zombie->state = zombie->state + 1;
    zombie->facing = 2;
    zombie->hitbox.left += pixels;
    zombie->hitbox.right += pixels;
    block * B = MapCollision(&zombie->hitbox);
    if(B != NULL)
    {
        zombie->hitbox.right = B->hitbox.left;
        if (zombie->damage < 5)
        {
            zombie->hitbox.left = zombie->hitbox.right - 31;
        } else {
            zombie->hitbox.left = zombie->hitbox.right - 97;
        }
        if(zombie->canJump)
        {
            ZombieJump(zombie, 15);
        }
    }
}

// Move o zumbi para a esquerda
void MoveLeftZombie(zombie * zombie, int pixels)
{
    zombie->state = zombie->state + 1;
    zombie->facing = 1;
    zombie->hitbox.left -= pixels;
    zombie->hitbox.right -= pixels;
    block * B = MapCollision(&zombie->hitbox);
    if(B != NULL)
    {
        zombie->hitbox.left = B->hitbox.right;
        if (zombie->damage < 5)
        {
            zombie->hitbox.right = zombie->hitbox.left + 31;
        } else {
            zombie->hitbox.right = zombie->hitbox.left + 97;
        }
        
        if(zombie->canJump)
        {
            ZombieJump(zombie, 15);
        }
    }

}

// O zumbi sofre gravidade
void ZombieGravity(zombie * zombie)
{
    MoveDownZombie(zombie, zombie->gravity);
    if(zombie->gravity < 20)
    {
        zombie->gravity++;
    }
}

// Move o zumbi até o jogador, caso a hitbox do zumbi colida com a do player, o player toma dano
void KnockbackZombie(zombie * zombie)
{
    if(zombie->knockbackSide == 1)
    {
        MoveLeftZombie(zombie, 7);
    } else {
        MoveRightZombie(zombie, 7);
    }
    MoveUpZombie(zombie, 12);
    ZombieGravity(zombie);
}

//Função com a inteligência artificial do zumbi
void MoveZombie(character * player, zombie * zombie)
{
    if(zombie->respawn)
    {
        --zombie->respawn;
        if(!zombie->respawn)
        {
            SpawnZombie(zombie, player);
        }
        return;
    }
    if(zombie->knockback)
    {
        KnockbackZombie(zombie);
        --zombie->knockback;
        zombie->canMove = 0;
        return;
    }
    if(zombie->canMove)
    {
        int left = player->hitbox.left;
        int right = player->hitbox.right;
        int top = player->hitbox.top;
        int bottom = player->hitbox.bottom;

        if(zombie->hitbox.right < right)
        {
            MoveRightZombie(zombie, 2);
        }
        if(zombie->hitbox.left > left)
        {
            MoveLeftZombie(zombie, 2);
        } 
        else
        {
            if (zombie->hitbox.bottom < top)
            {
                zombie->hitbox.bottom += 5;
                block * B = MapCollision(&zombie->hitbox);
                if (Collision(&zombie->hitbox, &B->hitbox))
                {
                    if (B->life == 0)
                    {
                        free(B);
                        DArrayRemove(&Map, B);
                        WriteArchive(&Map);
                    }
                    B->life -= 1;
                }
                zombie->hitbox.bottom -= 5;
            }
            else if (zombie->hitbox.top < bottom)
            {

            }
            
        }
    }
    ZombieGravity(zombie);
}

//Função para dar dano ao zumbi, certificando a sua morte
void DamageZombie(zombie * Zombie, int Damage)
{
    Zombie->life -= Damage;
    if(Zombie->life <= 0)
    {
        MoveDownZombie(Zombie, 1000);
        Zombie->respawn = 150;
    }
}

//Espadada no zumbi
int Slash(zombie * Zombie, character * Player)
{
    RECT Damage;
    Damage.top = Player->hitbox.top;
    Damage.bottom = Player->hitbox.bottom;
    if(Player->facing == 1)
    {
        Damage.right = Player->hitbox.left;
        Damage.left = Damage.right - 64;
    }else
    {
        Damage.left = Player->hitbox.right;
        Damage.right = Damage.left + 64;
    }
    if(Collision(&Zombie->hitbox, &Damage))
    {
        DamageZombie(Zombie, Player->inventory[Player->mainSlot].damage);
        return (Zombie->hitbox.left + 16) - (Player->hitbox.left + 15);
    }
    return 0;
}

//Dá tiro de doze
int EstragarVelorio(zombie * Zombie, character * Player, POINT Mouse)
{
    RECT Damage;
    Damage.top = Mouse.y - 16;
    Damage.bottom = Mouse.y + 15;
    Damage.left = Mouse.x - 16;
    Damage.right = Mouse.x + 15;
    if(Collision(&Zombie->hitbox, &Damage))
    {
        DamageZombie(Zombie, Player->inventory[Player->mainSlot].damage);
        return (Zombie->hitbox.left + 16) - (Player->hitbox.left + 15);
    }
    return 0;
}

void CenterPlayer(character * Player, zombie * Zombie)
{
    RECT ClientRect;
    GetClientRect(Ghwnd, &ClientRect);
    ClientRect.left = (ClientRect.right - ClientRect.left) >> 1;
    ClientRect.top = (ClientRect.bottom - ClientRect.top) >> 1;

    ClientRect.left = ClientRect.left - 16;
    ClientRect.right = ClientRect.left + 31;
    ClientRect.top = ClientRect.top - 32;
    ClientRect.bottom = ClientRect.top + 63;

    mapax -= ClientRect.left - Player->hitbox.left;
    mapay += ClientRect.top - Player->hitbox.top;

    MoveDownZombie(Zombie, ClientRect.top - Player->hitbox.top);
    MoveRightZombie(Zombie, ClientRect.left - Player->hitbox.left);

    Player->hitbox = ClientRect;
}