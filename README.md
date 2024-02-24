[![AMX MOD X](https://badgen.net/badge/Powered%20by/AMXMODX/0e83cd)](https://amxmodx.org)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

<h1 align="center">
  <a href="https://github.com/FEDERICOMB96/amxx-multimod-manager/releases"><img src="https://github.com/FEDERICOMB96/amxx-multimod-manager/assets/41979395/7ae51842-e586-4037-b4e6-6a61ad2186f9" width="900px" alt="Multimod Manager CS"></a>
</h1>

<p align="center">
    <a href="https://github.com/FEDERICOMB96/amxx-multimod-manager/releases/latest">
    <img src="https://img.shields.io/github/downloads/FEDERICOMB96/amxx-multimod-manager/total?label=Download%40latest&style=flat-square&logo=github&logoColor=white"
         alt="Build status">
    <a href="https://github.com/FEDERICOMB96/amxx-multimod-manager/actions">
    <img src="https://img.shields.io/github/actions/workflow/status/FEDERICOMB96/amxx-multimod-manager/build.yml?branch=master&style=flat-square&logo=github&logoColor=white"
         alt="Build status">
    <a href="https://github.com/FEDERICOMB96/amxx-multimod-manager/releases">
    <img src="https://img.shields.io/github/v/release/FEDERICOMB96/amxx-multimod-manager?include_prereleases&style=flat-square&logo=github&logoColor=white"
         alt="Release"><br>
    <a href="https://www.amxmodx.org/downloads-new.php?branch=master">
    <img src="https://img.shields.io/badge/AMXMODX-%3E%3D1.10.0.5461-blue?style=flat-square"
         alt="AMXModX dependency">
    <a href="https://github.com/dreamstalker/rehlds/releases/tag/3.13.0.788">
    <img src="https://img.shields.io/badge/ReHLDS-%3E%3D3.13.0.788-blue?style=flat-square"
         alt="ReHLDS dependency">
    <a href="https://github.com/s1lentq/ReGameDLL_CS/releases/tag/5.26.0.668">
    <img src="https://img.shields.io/badge/ReGameDLL-%3E%3D5.26.0.668-blue?style=flat-square"
         alt="ReGameDLL dependency">
    <a href="https://github.com/s1lentq/reapi/releases/tag/5.24.0.300">
    <img src="https://img.shields.io/badge/ReAPI-%3E%3D5.24.0.300-blue?style=flat-square"
         alt="ReAPI dependency">
</p>

- [English](#english)
- [Español](#español)

## English

Multimod plugin for CS 1.6 / CS:CZ

<p align="center">
  <a href="#requirements">Requirements</a> •
  <a href="#installation">Installation</a> •
  <a href="#updating">Updating</a> •
  <a href="#downloads">Downloads</a> •
  <a href="#features">Features</a> •
  <a href="#wiki">Wiki</a> •
  <a href="#contributing">Contributing</a> •
  <a href="#credits">Credits</a>
</p>

## Requirements:
- ReHLDS installed.
- ReGameDLL installed.
- AMXModX installed.
    - Installed ReAPI module (required).

[ReSemiclip compatibility](https://github.com/rehlds/resemiclip) >= 2.3.9 ✔️

## Installation:
- [Download the latest](https://github.com/FEDERICOMB96/amxx-multimod-manager/releases/latest) stable version from the release section.
- Extract the `addons` folder inside the `cstrike` folder of the ReHLDS server.

## Updating:
- Put the new plugin and lang-file (`plugins/*.amxx` & `data/lang/*.txt`) into `amxmodx/` folder on the ReHLDS server.
- Restart the server (command `restart` or change the map).
- Make sure that the version of the plugin are up to date with the command `amxx list`.

## Downloads:
- [Release builds](https://github.com/FEDERICOMB96/amxx-multimod-manager/releases)
- [Dev builds](https://github.com/FEDERICOMB96/amxx-multimod-manager/actions/workflows/build.yml)

## Features:
- Configurations through JSON file (friendly)
- Votemod
- Votemap
- Nominations (Mods and Maps)
- Rock The Vote
- Recently played mods and maps
- Say commands _(All say commands can be found [here](https://github.com/FEDERICOMB96/amxx-multimod-manager/wiki/Say-commands))_
- Admin commands (custom votes, manage, force votemod)
- `API` natives and forwards _(All detailed information is [here](https://github.com/FEDERICOMB96/amxx-multimod-manager/wiki/API))_

## Wiki:
Do you **need some help**? Check the _articles_ from the [wiki](https://github.com/FEDERICOMB96/amxx-multimod-manager/wiki).

## Contributing:
Got **something interesting** you'd like to **share**? Open a PR and I will be happy to review it.

## Credits:
Thank the creators of AMXModX, ReHLDS, ReGameDLL and ReAPI. And also to the following people who have contributed to the project and helps me in developing and testing this system.
- [metita](https://github.com/metita)
- [Mía](https://github.com/Mia2904)
- [Totopizza](https://github.com/oaus)
- [Roccoxx](https://github.com/Roccoxx)
- [r0ma](https://github.com/francoromaniello)
- [Maxi605](https://github.com/Maxi605)
- [dystopm](https://github.com/dystopm)

***

## Español

Plugin de Multimod para CS 1.6 / CS:CZ

<p align="center">
  <a href="#requerimientos">Requerimientos</a> •
  <a href="#instalación">Instalación</a> •
  <a href="#actualizando">Actualizando</a> •
  <a href="#descargas">Descargas</a> •
  <a href="#características">Características</a> •
  <a href="#wiki">Wiki</a> •
  <a href="#contribuyendo">Contribuyendo</a> •
  <a href="#créditos">Créditos</a>
</p>

## Requerimientos:
- ReHLDS instalado.
- ReGameDLL instalado.
- AMXModX instalado.
    - Módulo ReAPI instalado (requerido).

[Compatibilidad con ReSemiclip](https://github.com/rehlds/resemiclip) >= 2.3.9 ✔️

## Instalación:
- [Descargue la última](https://github.com/FEDERICOMB96/amxx-multimod-manager/releases/latest) versión estable desde la sección de lanzamientos.
- Extraiga la carpeta `addons` dentro de la carpeta `cstrike` del servidor ReHLDS.

## Actualizando:
- Coloque el nuevo plugin y el archivo de idioma (`plugins/*.amxx` & `data/lang/*.txt`) en la carpeta `amxmodx/` en el servidor ReHLDS.
- Reinicie el servidor (comando `restart` o cambie el mapa).
- Asegúrese de que la versión del plugin esté actualizada con el comando `amxx list`.

## Descargas:
- [Versiones estables](https://github.com/FEDERICOMB96/amxx-multimod-manager/releases)
- [Versiones de desarrollo](https://github.com/FEDERICOMB96/amxx-multimod-manager/actions/workflows/build.yml)

## Características:
- Configuraciones mediante archivo JSON (amigable)
- Votación de Modos
- Votación de Mapas
- Nominaciones (Modos and Mapas)
- Rock The Vote
- Modos y mapas recientemente jugados
- Comandos de Chat _(Todos los comandos de chat se pueden encontrar [aquí](https://github.com/FEDERICOMB96/amxx-multimod-manager/wiki/Say-commands))_
- Comandos de Administración (votaciones personalizadas, configuracion, forzar una votacion de modo)
- `API` natives y forwards _(Toda la información detallada se encuentra [aquí](https://github.com/FEDERICOMB96/amxx-multimod-manager/wiki/API))_

## Wiki
Necesitas **ayuda**? Consulte los _artículos_ de la [wiki](https://github.com/FEDERICOMB96/amxx-multimod-manager/wiki).

## Contribuyendo
¿Tienes **algo interesante** que te gustaría **compartir**? Abra un PR y estaré encantado de revisarlo.

## Créditos:
Gracias a los creadores de AMXModX, ReHLDS, ReGameDLL y ReAPI. Y también a las siguientes personas que han contribuido al proyecto y me ayudan a desarrollar y probar este sistema.
- [metita](https://github.com/metita)
- [Mía](https://github.com/Mia2904)
- [Totopizza](https://github.com/oaus)
- [Roccoxx](https://github.com/Roccoxx)
- [r0ma](https://github.com/francoromaniello)
- [Maxi605](https://github.com/Maxi605)
- [dystopm](https://github.com/dystopm)
