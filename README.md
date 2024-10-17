# Byte Wars - Unreal Engine

## Overview

Byte Wars is the official tutorial game for AccelByte Gaming Services (AGS). It is intended to act as a sample project that can be used as a reference on the best practices to integrate our services into your game. We created Byte Wars from scratch as a fully functional offline game. This offline game was then brought online with the power of AccelByte’s platform by adding different services from each of our service areas like access, play and engagement. Every tutorial module walks you through a step by step guide to add a specific feature to Byte Wars which you can then translate into your own game.

## Prerequisites

* Use **Unreal Engine** version 5.1.0 source build from [Unreal Engine on Github](https://www.unrealengine.com/en-US/ue-on-github).

## Branches

Byte Wars published the source code in two branches:
* **main branch** is the vanila version game source code without any submodule and will in use for [Tutorial Module: Initial Setup](https://docs.accelbyte.io/gaming-services/tutorials/byte-wars/unreal-engine/learning-modules/general/module-initial-setup/).
* **tutorialmodules branch** is the branch that has the AccelByte's Plugins and all the Byte Wars tutorial modules with feature flag. 

## Clone Byte Wars

### Main Branch

Run the following git command to clone the `main` branch.
```batch
git clone git@github.com:AccelByte/accelbyte-unreal-bytewars-game.git
```
### Tutorial Modules Branch

The `tutorialmodules` branch has several plugins set as submodules:
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

## Run Byte Wars Offline (Main Branch)

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
### Connect Game Client to Game Server Locally

1. Run both game client and game server.
2. On game client, make sure it's in main menu, open command prompt using ` (tilde key on keyboard)
3. Then run the following command to connect to game server.
   ```batch
   open 127.0.0.1:7777/Game/ByteWars/Maps/MainMenu/MainMenu
   ```

## Run Byte Wars Online (Tutorial Modules Branch)

Follow along Byte Wars [Learning Paths](https://docs.accelbyte.io/gaming-services/tutorials/byte-wars/unreal-engine/learning-paths/). We suggest you to start with the [Login with Device ID and Steam path](https://docs.accelbyte.io/gaming-services/tutorials/byte-wars/unreal-engine/learning-paths/authentication/unreal-path-login-device-id-and-steam/) if you're unsure where to start.