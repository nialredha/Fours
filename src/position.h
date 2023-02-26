#ifndef POSITION_H
#define POSITION_H

typedef struct Position
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
} Position_Ref;

#endif // POSITION_H
