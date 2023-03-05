#ifndef POSITION_H
#define POSITION_H

typedef struct 
{
    int x, y; 
} Position;

typedef enum 
{
    TOP, 
    BOTTOM, 
    LEFT,
    RIGHT, 
    CENTER,
} Reference;

typedef struct
{
    Reference x;
    Reference y;
} Reference_Point;


#endif // POSITION_H
