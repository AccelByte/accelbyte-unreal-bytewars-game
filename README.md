# Byte Wars AMS - Unreal Engine

## Overview

Byte Wars AMS is the official tutorial game for AccelByte Multiplayer Servers (AMS). It is intended to act as a sample project that can be used as a reference on the best practices to integrate our services into your game. We created Byte Wars from scratch as a fully functional offline game. This offline game was then brought online with the power of AccelByte’s platform by adding different services from each of our service areas like access, play and engagement. Every tutorial module walks you through a step by step guide to add a specific feature to Byte Wars which you can then translate into your own game.

This version of Byte Wars, act as a sample project for AccelByte Multiplayer Servers (AMS) without AccelByte Gaming Services (AGS). This is intended for developer who want to use AMS with other services. In this sample, we are using a sample matchmaking service that can be run locally alongside Byte Wars.

## Prerequisites

- Use **Unreal Engine** version 5.2.1 source build from [Unreal Engine on Github](https://www.unrealengine.com/en-US/ue-on-github).
- Sample matchmaking service from [AMS Samples](https://github.com/AccelByte/ams-samples)

## Clone Byte Wars

### AMS Branch

Run the following git command to clone the `tutorialmodules-ams` branch:

```batch
git clone --branch tutorialmodules-ams --recursive git@github.com:AccelByte/accelbyte-unreal-bytewars-game.git
```

The `tutorialmodules-ams` branch has one plugin set as submodule:

- [AccelByte Unreal SDK](https://github.com/AccelByte/accelbyte-unreal-sdk-plugin) under `Plugins/AccelByte/AccelByteUe4Sdk`.

To clone the repository and checkout the submodule at the same time, run the following command:

```batch
    git clone --branch <branch-name> --recursive git@github.com:AccelByte/accelbyte-unreal-bytewars-game.git
```

## Compile Byte Wars

1. Right click on AccelByteWars.uproject, select unreal engine version, then choose unreal engine version 5.2.1 source build that you have already installed.
2. Open AccelByteWars.sln generated from step number 1, using your prefered IDE.
3. Compile the game project using Development Editor - Win64.

## Run Byte Wars AMS

### Game Client

#### Run via Editor with Command Line

1. Open terminal.
2. Enter the following command:
    ```batch
    "<path_to_ue>/Engine/Binaries/Win64/UnrealEditor.exe" "<path_to_project>/AccelByteWars.uproject" -game
    ```

#### Run via PIE

1. Open unreal editor by double clicking on AccelByteWars.uproject or run unreal editor via IDE (Development Editor - Win64).
2. Click on PIE button to run the game.

#### Run via Packaged Game Client

1. Open unreal editor.
2. Package game sever platform Windows, Development config, AccelByteWars as the target.
3. On build complete, then run the package game client.


### Game Server

#### Run via Editor with Command Line

1. Open terminal.
2. Enter the following command:
    ```batch
    "<path_to_ue>/Engine/Binaries/Win64/UnrealEditor.exe" "<path_to_project>/AccelByteWars.uproject" -server
    ```

#### Run via Packaged Game Server

1. Open unreal editor.
2. Package game sever platform Windows, Development config, AccelByteWarsServer as the target.
3. On build complete, run the package game server using the following command:
   ```batch
    AccelByteWarsServer.exe -server -log
   ```

## How to connect the game to the Sample Matchmaking

1. Run the sample matchmaking project.
2. Open terminal and run the game by using the following command:
    - With editor:
        ```batch
        "<path_to_ue>/Engine/Binaries/Win64/UnrealEditor.exe" "<path_to_project>/AccelByteWars.uproject" -game -CustomMatchmakingUrl="<ip>:<port>"
        ```
    - With packaged game server:
        ```batch
        AccelByteWars.exe -CustomMatchmakingUrl="<ip>:<port>"
        ```
    Set the <ip>:<port> to `127.0.0.1:8080` if you are running the sample matchmaking in the same machine.
3. Once the game opened and currently in the main menu, go to **Custom Matchmaking** and click **Start Matchmaking**. The game will connect to the sample matchmaking service and you should see a loading screen.

## AMS Integration Tutorial

Follow along [Byte Wars AMS](https://docs.accelbyte.io/gaming-services/tutorials/byte-wars-ams/unreal-engine/) to learn more about the integration.
