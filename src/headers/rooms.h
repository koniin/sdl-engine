#ifndef ROOMS_H
#define ROOMS_H

#include <string>

// Bytepath creator analysis of Nuclear throne
// MainMenu -> Play -> CharacterSelect -> Game -> MutationSelect -> Game -> .... 
// MainMenu - the first menu with options etc
// Play - where you select difficulty and mode (normal, daily, weekly)
// CharacterSelect - where you select your character and starting mutation
// Game - where you play
// MutationSelect - after game screen where you select a mutation

/*
 
 Generalized:
 MainMenu -> NewGame -> Game -> AfterGame -> Game -> ...
 
 Rooms:
 MainMenu - as above
 NewGame - has three states -> used to create the game state initial options
    - select difficulty
    - select mode 
    - select character
 Game - has three states -> used to alter the game state
    - load/generate level - uses game state as input
    - play - uses the generated data to play the game
    - death - shows that you died and uses game state to show other info
 AfterGame
    - show info on completed level
    - select something with your character
    - select next map? - I think we start with linear progression
    - show your path through the atlas?

*/

// we need to store something between rooms then ( GameState (state for Game, see it makes sense) )
// - Seed for generation
// - Character - stats/weapons etc
// - Score
// - Current level
// - completed levels
// - difficulty
// - Statistics
//      - targets killed
//      - what killed you
//      - how many bullets fired
// - Atlas if any - with progression
// -- others?

enum Rooms {
    MainMenu = 0,
    NewGame = 1,
    Game = 2,
    AfterGame = 3,
    NO_ROOM
};

void room_goto(const Rooms room);
void room_update();
void room_render();
void room_render_ui();
void room_load_all();
void room_unload_all();

#endif