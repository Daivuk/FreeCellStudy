# FreeCell implenentation study
Trying various implementations and coding paradigm for a FreeCell game.

The different implementations are stored in separate branches.

|Branch|Description|Language|Frameworks|
|-|-|-|-|
|oop|Object Oriented Programming|C++|[onut](https://github.com/Daivuk/onut)|
|c|Plain C|C|[SDL](https://github.com/SDL-mirror/SDL.git)|
|dod|Data Oriented Design|C|[SDL](https://github.com/SDL-mirror/SDL.git)|
|ecs|Entity Component Systems|C++|[SDL](https://github.com/SDL-mirror/SDL.git)|

Features that we try to have in each implementation:
- Single click card to auto-move it.
- Animated card when moving from one pile to another
- Hold and drag to new pile
- Show moves and time
- On victory, compare moves and time against previous bests
- Store previous bests on disk
- Ctrl+Z to undo up to 200 moves
- Ctrl+N for new game
- F5 to restart current game with the same seed
- Auto solve when all stacks can be brought to home directly

![C++](images/cpp.jpg)
![C](images/c.jpg)
