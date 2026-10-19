# OS server-client communications template

This repo can serve as some kind of `template` for possible server-client communications

May not be the best, but could be useful

> [!WARNING]
> VERY INCOMPLETE!!!
>
> Will mainly be used for OS project

### Instructions for use
##### Compiling
one can just run `make -j` and it will build all binaries and link such dependencies

##### Game State
both applications share/use a gameState in `include/game_state.h`

##### Client Side
one can handle client-side packet communications in
`src/client.cpp` -> `handleGameState(...)`

##### Server Side
one can handle server-side packet communications in
`src/server.cpp` -> `handleGameState(...)`

