/*

Essa header possui todos os imports que precisamos, para garantir que não vai dar errado

*/


#ifndef IMPORTS_H
#define IMPORTS_H

#ifndef UNICODE
#define UNICODE
#endif

int mapax = 0;
int mapay = 800;
int DirtLife = 3, LogLife = 4, StoneLife = 4, LeavesLife = 1, GrassLife = 4;

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <math.h>
#include "VirtualKeyCodes.h" //Arquivo com os códigos de teclas não definidas no Windows.h
#include "Stack&Queue.h"
#include "DynamicArray.h"
#include "LinkedStackQueue.h"
#include "LinkedList.h"
#include "QuickSort.h"
#include "blocks.h"
#include "items.h"
#include "characters.h"

DArray Map;//Mapa criado aqui para ser acessado em qualquer lugar do programa


#endif