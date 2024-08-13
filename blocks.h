#ifndef BLOCKS
#define BLOCKS
#include <stdio.h>
#include "imports.h"

char * MapPath = "Map.txt";

// Estrutura dos blocos, especificando com posição e tipo.
typedef struct block
{
    RECT hitbox;//Retângulo usado para colisão e desenho
    int x;//posições simplificadas
    int y;
    int type;
    int life;
    wchar_t * img;//Caminho para a imagem que deve ser desenhada
} block;

// Função de definição da textura e colisão dos blocos
void BlockDefine(block * B)
{
    B->hitbox.left = (B->x*32) - mapax;
    B->hitbox.top = 977 - (B->y*32) + mapay;
    B->hitbox.right = B->hitbox.left + 32;
    B->hitbox.bottom = B->hitbox.top + 32;
    switch (B->type)
    {
    case 1:
        switch (B->life)
        {
            case 1:
                B->img = L"GramaQ3";
                break;            

            case 2:
                B->img = L"GramaQ2";
                break;

            case 3:
                B->img = L"GramaQ1";
                break;
            
            case 4:
                B->img = L"Grama";
                break;
            
            default:
                break;
        }
        break;

    case 2:
        switch (B->life)
        {
            case 1:
                B->img = L"TerraQ2";
                break;

            case 2:
                B->img = L"TerraQ1";
                break;

            case 3:
                B->img = L"Terra";
                break;
                
            default:
                break;
        }
        break;

    case 3:
        switch (B->life)
        {
            case 1:
                B->img = L"TroncoQ3";
                break;            

            case 2:
                B->img = L"TroncoQ2";
                break;

            case 3:
                B->img = L"TroncoQ1";
                break;
            
            case 4:
                B->img = L"Tronco";
                break;
            
            default:
                break;
        }
        break;

    case 4:
        B->img = L"Folhas";
        B->life = 1;
        break;

    case 5:
        B->img = L"Stone";
        B->life = 3;
        break;

    default:
        break;
    }
}

void DefineMap(DArray *lista)
{
    for(int i = 0; i < lista->Size; i++)
    {
        block * B = lista->List[i];
        BlockDefine(B);
    }
}

// função para criar o mapa aleatoriamente
void CreateArchive()
{
    FILE * map = fopen("Map.txt", "w");
    int camada = rand()%8 + 6;
    int arvore = 0, arvtam = 0;
    int ypedra = camada - (rand()%2 + 5);
    
    for(int x = -150; x <= 150; x++)
    {
        if(arvore == 0 && rand()%10 == 1) // criação aleatoria da arvore, 10% de chance de criar uma arvore caso o bloco anterior não tenha uma arvore
        {
            arvore = camada + 1;
            arvtam = rand()%6 + 3; // tamanho aleatorio da arvore
            for(int arvh = arvore; arvh < arvore + arvtam; arvh++) // criação dos troncos em linha reta para cima de acordo com o tamanho da arvore
            {
                fprintf(map, "%d,%d,%d\n", x, arvh, 3); // escreve o local do bloco no txt do mapa
            }
            // criação das folhas da arvore em volta do tronco de acordo com o tamanho da arvore
            for(int folhay = arvore + arvtam; folhay < arvore + arvtam * 2; folhay++)
            {
                for(int folhax = (x - arvtam / 2) + ((folhay - arvore - arvtam) / 2); folhax <= (x + arvtam / 2) - ((folhay - arvore - arvtam) / 2); folhax++)
                {
                    fprintf(map, "%d,%d,%d\n", folhax, folhay, 4); // escreve o local do bloco no txt do mapa
                }
            }
        }
        else if(arvore != 0) // caso o bloco anterior possua uma arvore retorna a variavel para 0
        {
            arvore = 0;
        }
        for(int y = camada; y >= -50; y--) // criação dos blocos do chão
        {
            fprintf(map, "%d,%d,%d\n", x, y, (y <= ypedra) ? 5 : (y == camada && arvore != camada + 1) ? 1 : 2); // escreve o local do bloco no txt do mapa e escolhe o tipo do bloco de acordo com a camada
        }
        camada += rand()%3 - 1; // aumenta ou diminui em 1 ou mantem a camada aleatoriamente
        if(camada < 6) // limita o minimo da camada que será gerada
        {
            camada = 6;
        }
        if(camada > 14) // limita o maximo da camada que será gerada
        {
            camada = 14;
        }
        ypedra += rand()%3 - 1; // aumenta ou diminui em 1 ou mantem a camada da pedra aleatoriamente
        if(ypedra > camada - 5) // limita o minimo da camada de pedra que será gerada
        {
            ypedra = camada - 5;
        }
        if(ypedra < camada - 7) // limita o maximo da camada de pedra que será gerada
        {
            ypedra = camada - 7;
        }
    }
    fclose(map); // finaliza a criação do mapa
}
/* 
Função de leitura do arquivo de mapa, fazemos a construção do mapa do Ternaria por meio da leitura desse arquivo.
*/ 
void ReadArchive(DArray *lista)
{
    int count = 0;
    FILE * File = fopen("Map.txt", "r");
    MapPath = "Map.txt";

    while(!feof(File))
    {
        ++count;
        block *bloco = malloc(sizeof(block));
        fscanf(File, "%i,%i,%i", &bloco->x, &bloco->y, &bloco->type);
                switch (bloco->type)
        {
            case 1:
                bloco->life = GrassLife;
                break;

            case 2:
                bloco->life = DirtLife;
                break;

            case 3:
                bloco->life = LogLife;
                break;

            case 4:
                bloco->life = LeavesLife;
                break;

            case 5:
                bloco->life = StoneLife;
                break;
            
            default:
                break;
        }
        BlockDefine(bloco);
        if((bloco->x*32) < -(5 * 32) + mapax || (bloco->x*32) > (60 * 32) + mapax || (bloco->y*32) > (30 * 32) + mapay || (bloco->y*32) < -(5 * 32) + mapay)
        {
            free(bloco);
        }
        else
        {
            DArrayAdd(lista, bloco);
        }
    }
    fclose(File);
}

// Lê o Arquivo do Mundo Senai
void ReadSenai(DArray *lista)
{
    int count = 0;
    FILE * File = fopen("Map Senai.txt", "r");
    MapPath = "Map Senai.txt";

    while(!feof(File))
    {
        ++count;
        block *bloco = malloc(sizeof(block));
        fscanf(File, "%i,%i,%i", &bloco->x, &bloco->y, &bloco->type);
        BlockDefine(bloco);
        DArrayAdd(lista, bloco);
    }
    fclose(File);
}

// Escreve no txt
// void WriteArchive(DArray *lista)
// {
//     int count = 0;
//     FILE * File = fopen(MapPath, "w");
//     for(int i = 0; i < lista->Size; ++i)
//     {
//         block * B = (block *) lista->List[i];
//         fprintf(File, "%i,%i,%i\n", B->x, B->y, B->type);
//     }
//     fclose(File);
// }

void AddBlockArchive(DArray *lista, block * B)
{
    FILE * File = fopen(MapPath, "r");
    FILE * File2 = fopen("Map2.txt", "w");
    fprintf(File2, "%i,%i,%i\n", B->x, B->y, B->type);
    while(!feof(File))
    {
        int x = 0, y = 0, type = 0;
        fscanf(File, "%i,%i,%i", &x, &y, &type);
        if(type != 0)
        {
            fprintf(File2, "%i,%i,%i\n", x, y, type);
        }
    }
    fclose(File);
    fclose(File2);
    File = fopen(MapPath, "w");
    File2 = fopen("Map2.txt", "r");
    while(!feof(File2))
    {
        int x = 0, y = 0, type = 0;
        fscanf(File2, "%i,%i,%i", &x, &y, &type);
        if(type != 0)
        {
            fprintf(File, "%i,%i,%i\n", x, y, type);
        }
    }
    fclose(File);
    fclose(File2);
}

void RemoveBlockArchive(DArray *lista, block * B)
{
    FILE * File = fopen(MapPath, "r");
    FILE * File2 = fopen("Map2.txt", "w");
    while(!feof(File))
    {
        int x = 0, y = 0, type = 0;
        fscanf(File, "%i,%i,%i", &x, &y, &type);
        if(type != 0)
        {
            fprintf(File2, "%i,%i,%i\n", x, y, type);
        }
    }
    fclose(File);
    fclose(File2);
    File = fopen(MapPath, "w");
    File2 = fopen("Map2.txt", "r");
    while(!feof(File2))
    {
        int x = 0, y = 0, type = 0;
        fscanf(File2, "%i,%i,%i", &x, &y, &type);
        if(!(B->type == type && B->x == x && B->y == y) && type != 0)
        {
            fprintf(File, "%i,%i,%i\n", x, y, type);
        }
    }
    fclose(File);
    fclose(File2);
}
#endif