# Byte Wars - Unreal Engine

## Overview

Byte Wars is the official tutorial game for AccelByte Gaming Services (AGS). It is intended to act as a sample project that can be used as a reference on the best practices to integrate our services into your game. We created Byte Wars from scratch as a fully functional offline game. This offline game was then brought online with the power of AccelByte’s platform by adding different services from each of our service areas like access, play and engagement. Every tutorial module walks you through a step by step guide to add a specific feature to Byte Wars which you can then translate into your own game.

## Prerequisites

* Use **Unreal Engine** version 5.1.0 source build from [unreal engine on github](https://www.unrealengine.com/en-US/ue-on-github).


## Clone Byte Wars

This repository has two main branches, master and online.
* **Master branch** is the vanila version game source code without any submodule.
* **Online branch** is the branch that has the AccelByte Online Subsystem submodule, which is used by another repository for Byte Wars tutorial.

### Master Branch

Just run the following git command to clone the game.
```batch
git clone git@bitbucket.org:accelbyte/accelbyte-unreal-bytewars-game.git
```
### Online Branch

This online branch has a submodule to [justice-ue4-oss](https://bitbucket.org/accelbyte/justice-ue4-oss/src/master/) at `Plugins/AccelByte`. To clone the repository and checkout the submodule at the same time, run the following command:
```batch
    git clone --branch online --recurse-submodules --remote-submodules git@bitbucket.org:accelbyte/accelbyte-unreal-bytewars-game.git
```
The `--recurse-submodules` flag will clone the submodule at the revision that was previously pushed to this repository. Adding `--remote-submodules` will then update the submodules to the latest of their tracked branch, in this case the `master` branch. This is important as while under development, not all submodule updates will result in an updated commit SHA for the submodule tracking. Using the remote flag will ensure that you are on the latest OSS plugin version.

Note: that since the OSS plugin module is set to track the `master` branch and will change a lot as a result, it would be helpful to make your `git pull` automatically check in changes from submodules. You can do this with the following config command:
```batch
    git config submodule.recurse true
```    
This will then try and update all submodules when you run `git pull`, keeping everything up to date with the single root pull command.

## Compile Byte Wars

1. Right click on AccelByteWars.uproject, select unreal engine version..  then choose unreal engine version 5.1.0 that you already installed.
2. Open AccelByteWars.sln generated from step number 1, using your prefered IDE.
3. Compile the game project using Development Editor - Win64.

## Run Byte Wars

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