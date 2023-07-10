[![AMX MOD X](https://badgen.net/badge/Powered%20by/AMXMODX/0e83cd)](https://amxmodx.org)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

# MultiMod Manager (Counter Strike 1.6)

## Colaboradores _(Collaborators)_:
- [FEDERICOMB](https://github.com/FEDERICOMB96)
- [metita](https://github.com/metita)
- [Mía](https://github.com/Mia2904)
- [Totopizza](https://github.com/oaus)
- [Roccoxx](https://github.com/Roccoxx)
- [r0ma](https://github.com/francoromaniello)

## Requerimientos _(Requirements)_:
- [AmxModX](https://github.com/alliedmodders/amxmodx) >= 1.9.0.5263
- [ReHLDS](https://github.com/dreamstalker/rehlds) >= 3.9.0.752-dev
- [ReGameDLL](https://github.com/s1lentq/ReGameDLL_CS) >= 5.20.0.516-dev
- [ReAPI](https://github.com/s1lentq/reapi) >= 5.19.0.217
- [ReSemiclip](https://github.com/rehlds/resemiclip) >= 2.3.9 (Opcional) _(Optional)_

## Características _(Features)_:
- Configuraciones mediante archivo JSON _(Configurations through JSON file)_
- Votación de Modos _(Votemod)_
- Votación de Mapas _(Votemap)_
- Nominaciones _(Nominations)_
- RTV _(Rock The Vote)_
- Comandos de Administración _(Admin commands)_
- Modos y mapas recientemente jugados _(Recently played mods and maps)_

<details>
<summary>Click para ampliar (Click to expand):</summary>
  
| Característica<br>_(Feature)_               | Descripción<br>_(Description)_                                     |
| :------------------------------------------ | :-------------------------------------------------------------- |
| Archivo JSON<br>_(JSON File)_               | `[es]` No existe límite de modos que puedan ser cargados y cada modo puede tener configuraciones diferentes, cvars, plugins y resemiclip.<br>`[en]` There is no limit of mods that can be loaded and each mode can have different configurations, cvars, plugins and resemiclip. |
| Votación de Modos<br>_(Votemod)_            | `[es]` Antes de finalizar cada mapa, se enviará una votación de modo de forma automática con modos al azar (o nominados) a todo el servidor para elegir el próximo. En caso de empate con 2 o más opciones, se realizará otra votación continuada para desempatar.<br>`[en]` Before finishing each map, a votemod will be sent automatically with random modes (or nominated) to the entire server to choose the next one. In case of a tie with 2 or more options, another continuous vote will be held to break the tie. |
| Votación de Mapas<br>_(Votemap)_            | `[es]` A continuación de la votación de modo, se enviará una votación de mapa de forma automática con mapas al azar (o nominados) a todo el servidor para elegir el próximo. En caso de empate con 2 o más opciones, se realizará otra votación continuada para desempatar.<br>`[en]` After the votemod, a votemap will be sent automatically with random maps (or nominated) to the entire server to choose the next one. In case of a tie with 2 or more options, another continuous vote will be held to break the tie. |
| Nominaciones<br>_(Nominations)_             | `[es]` Es posible nominar modos y mapas disponibles para que puedan aparecer en las votaciones. Cada jugador puede nominar solamente un modo y un mapa a la vez y pueden ser removidas por el mismo. Al desconectarse se eliminará su nominación automáticamente.<br>`[en]` It is possible to nominate available modes and maps so that they can appear in the votes. Each player can nominate only one mode and one map at a time and can be removed by the same. When disconnecting, your nomination will be automatically removed. |
| RTV<br>_(Rock The Vote)_                    | `[es]` Se podrá hacer uso del famoso "rtv" para solicitar una votación por los usuarios. La misma podrá contener modos/mapas nominados por los jugadores o seleccionados al azar.<br>`[en]` The famous "rtv" can be used to request a vote by the users. It can contain mods / maps nominated by the players or selected at random. |
| Comandos de Administración<br>_(Admin commands)_ | `[es]` <br>* Puede administrar los modos cargados, ya sea activandolos o desactivandolos.<br>* Puede seleccionar un modo y un mapa del modo y cambiarlo.<br>* Puede seleccionar un modo y enviar una votación de mapas con mapas seleccionados de ese modo.<br>* Puede iniciar una votación de modos.<br> * Puede eliminar modos/mapas jugados recientemente.<br> * Admin al desconectarse su votación será cancelada.<br>`[en]` <br>* You can manage the loaded mods, either by activating or deactivating them.<br>* You can select a mod and a map of the mod and change it.<br>* You can select a mod and send a map vote with maps selected from that mod.<br>* You can start a mod vote.<br>* You can delete recently played mods/maps.<br>* Admin when disconnecting your vote will be canceled. |
| Compatiblidad con ReSemiclip<br>_(ReSemiclip Compatibility)_                    | `[es]` Es posible configurar ReSemiclip para cada modo con distintas configuraciones.<br>`[en]` It is possible to configure ReSemiclip for each mode with different configurations |

</details>

## Comandos de Chat _(Say commands)_:
| Comando<br>_(Command)_                      | Descripción<br>_(Description)_                                     |
| :------------------------------------------ | :-------------------------------------------------------------- |
| recentmods                                  | `[es]` Abre un listado de modos recientemente jugados.<br>`[en]` Opens a list of recently played mods.                  |
| recentmaps                                  | `[es]` Abre un listado de mapas recientemente jugados del modo actual.<br>`[en]` Opens a list of recently played maps of the current mod.  |
| currentmod                                  | `[es]` Muestra el modo actual.<br>`[en]` Shows the current mod. |
| currentmap                                  | `[es]` Muestra el mapa actual.<br>`[en]` Shows the current map. |
| nextmod                                     | `[es]` Muestra el siguiente modo votado.<br>`[en]` Shows the next voted mod. |
| nextmap                                     | `[es]` Muestra el siguiente mapa votado.<br>`[en]` Shows the next voted map. |
| timeleft                                    | `[es]` Muestra el tiempo restante o rondas restantes.<br>`[en]` Shows the remaining time or remaining rounds. |

## API (natives):
- mm_get_mod_id()
- mm_get_mod_name(const iModId, szOutput[], const iLen)
- mm_get_mod_tag(const iModId, szOutput[], const iLen)
- mm_get_nextmod_id()
- mm_get_nextmod_name(szOutput[], const iLen)
- mm_force_votemod()

Toda la información detallada se encuentra en el archivo `include\multimod_manager_natives.inc` _(All detailed information is in the `include\multimod_manager_natives.inc` file)_

## Compilación _(Compilation)_:
`[es]` Descargar el repositorio a una carpeta cualquiera, abrir el archivo multimod_manager.sma y compilar desde ahí con tu editor de texto preferido (Sublime Text, VSCode recomendados). No es necesario copiar los archivos .inc a la carpeta includes de donde tengas el compilador, pero es recomendable si compilas desde ahí.

`[en]` Download the repository to any folder, open the multimod_manager.sma file and compile from there with your favorite text editor (Sublime Text, VSCode recommended). It is not necessary to copy the .inc files to the includes folder where you have the compiler, but it is recommended if you compile from there.
