# Byte Wars - Unreal Engine

## Overview

Byte Wars is the official tutorial game for AccelByte Gaming Services (AGS). It is intended to act as a sample project that can be used as a reference on the best practices to integrate our services into your game. We created Byte Wars from scratch as a fully functional offline game. This offline game was then brought online with the power of AccelByte’s platform by adding different services from each of our service areas like access, play and engagement. Every tutorial module walks you through a step by step guide to add a specific feature to Byte Wars which you can then translate into your own game.

## Prerequisites

* Use **Unreal Engine** version 5.1.0 source build from [unreal engine on github](https://www.unrealengine.com/en-US/ue-on-github).


## Clone Byte Wars

This repository has two main branches, master and online.
* **Main branch** is the vanila version game source code without any submodule.
* **Online branch** is the branch that has the AccelByte's Plugins, which are used by the Byte Wars Tutorial Module 1.
* **tutorial/online-module.2** is the branch based on `online` branch and also has the required resources for user to learn Byte Wars Tutorial Module 2.
* **tutorial/online-module.3** is the branch based on `module 2` branch and also has the required resources for user to learn Byte Wars Tutorial Module 3.

### Main Branch

Just run the following git command to clone the game.
```batch
git clone git@github.com:AccelByte/accelbyte-unreal-bytewars-game.git
```
### Online and Tutorial Module Branches

The online branch, the tutorial module 2 branch, and the tutorial module 3 branch, they have several plugins set as submodules:
* [AccelByte Unreal Online Subsystem](https://github.com/AccelByte/accelbyte-unreal-oss) under `Plugins/AccelByte/OnlineSubsystemAccelByte`.
* [AccelByte Unreal SDK](https://github.com/AccelByte/accelbyte-unreal-sdk-plugin) under `Plugins/AccelByte/AccelByteUe4Sdk`.
* [AccelByte Network Utilities](https://github.com/AccelByte/accelbyte-unreal-network-utilities) under `Plugins/AccelByte/AccelByteNetworkUtilities`.

To clone the repository and checkout the submodule at the same time, run the following command:

```batch
    git clone --branch <branch-name> --recursive git@github.com:AccelByte/accelbyte-unreal-bytewars-game.git
```

## Compile Byte Wars

1. Right click on AccelByteWars.uproject, select unreal engine version..  then choose unreal engine version 5.1.0 that you already installed.
2. Open AccelByteWars.sln generated from step number 1, using your prefered IDE.
3. Compile the game project using Development Editor - Win64.

## Run Byte Wars (from Main branch)

### Game Client

#### Run via PIE

1. Open unreal editor by double clicking on AccelByteWars.uproject or run unreal editor via IDE (Development Editor - Win64).
2. Click on PIE button to run the game.

#### Run via Packaged Game Client

1. Open unreal editor.
2. Package game sever platform Windows, Development config, AccelByteWars as the target.
3. On build complete, then run the package game client.


### Game Server

#### Run via Packaged Game Server

1. Open unreal editor.
2. Package game sever platform Windows, Development config, AccelByteWarsServer as the target.
3. On build complete, then run the package game server using the following command.
   ```batch
    AccelByteWarsServer.exe -server -log
   ```
## Connect Game Client to Game Server

1. Run both game client and game server.
2. On game client, make sure it's in main menu, open command prompt using ` (tilde key on keyboard)
3. Then run the following command to connect to game server.
   ```batch
   open 127.0.0.1:7777/Game/ByteWars/Maps/MainMenu/MainMenu
   ``` 

## Byte Wars Launch Arguments

### Game Server

1. Specify game mode to be used
   ```batch
   -GameMode=<game_mode_code_name>
   ```
   By default, Game server will use the first index of DT_GameModes.

2. Specify Game Dedicated Server's shutdown delay upon game finished.
   ```batch
   -ShutdownOnFinishedDelay=<delay_in_secs>
   ```
   Triggered on Dedicated Server only, right after game ends, when the Game Over UI shown in client.
   Default: 30.

3. Specify Game Dedicated Server's shutdown delay upon game finished.
   ```batch
   -ShutdownOnOneTeamOrLessDelay=<delay_in_secs>
   ```
   Triggered on Dedicated Server only, whenever there's one or less team.
   Default: 30.