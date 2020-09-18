# Snake

Full Area:80*25

Game Area:48*23

蛇方向

0右 1左 2上 3下

```
/================================================|=============================\
|                                                |                             |
|                                                |  _____             _        |
|                                                | / ____|           | |       |
|                                                || (___  _ __   __ _| | _____ |
|                                                | \___ \| '_ \ / _` | |/ / _ \|
|                                                | ____) | | | | (_| |   <  __/|
|                                                ||_____/|_| |_|\__,_|_|\_\___||
|                                                |                             |
|                                                |                             |
|                                                |      Snake1:   points       |
|                                                |                             |
|                 Game Area                      |      Snake2:   points       |
|                                                |                             |
|                                                |      Snake3:   points       |
|                                                |                             |
|                                                |                             |
|                                                |      -----Control-----      |
|                                                |  C/X for Create/Kill Snake  |
|                                                |       Q for Quit Game       |
|                                                |                             |
|                                                |    W /S /A /D for Snake1    |
|                                                |    Arrow keys for Snake2    |
|                                                |    8 /5 /4 /6 for Snake3    |
\================================================|=============================/
```
Snake:---
Snake:123
Snake:DEAD

C/X for Create/Kill Snake

Q for Quit Game
Control
W/S/A/D for Snake1
Arrow keys for Snake2
8/5/4/6 for Snake3

```
/================================================|
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                 Game Area                      |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
\================================================|
|     _    _      _                              |
|    | |  | |    | |                             |
|    | |  | | ___| | ___ ___  _ __ ___   ___     |
|    | |/\| |/ _ \ |/ __/ _ \| '_ ` _ \ / _ \    |
|    \  /\  /  __/ | (_| (_) | | | | | |  __/    |
|     \/  \/ \___|_|\___\___/|_| |_| |_|\___|    |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |
|                                                |

"        Congratulations on discovering          ",
"     this game hidden in the Linux kernel.      ",
"         This is a snake game that can          ",
"        be played by up to three players.       ",
"                                                ",
"     Sincerely hope you can enjoy this game.    ",
"                          ---development team   ",
"                                                ",
"                                                ",
"           Press C to start the game.           ",

```



## 游戏状态转换

0态 游戏未开始
1态 游戏开始（0条蛇）
2态 游戏开始（1条蛇）
3态 游戏开始（2条蛇）
4态 游戏开始（3条蛇）

## 游戏界面

- 界面
- 游戏区域
  - 渲染三条蛇  
- 信息区

## 游戏对象管理

共三条蛇，每条蛇对应固定的操纵方式

- 增加蛇：遍历蛇列表，找到序号最小的空闲蛇
- 删除蛇
  - x键删除：遍历蛇列表，找到序号最大的空闲蛇；清理此蛇；标记为未创建；状态转换-1态
  - 蛇死亡：清理此蛇；标记未死亡；状态转换-1态
