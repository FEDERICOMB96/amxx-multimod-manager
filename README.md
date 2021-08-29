[![AMX MOD X](https://badgen.net/badge/Powered%20by/AMXMODX/0e83cd)](https://amxmodx.org)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

# MultiMod Manager

## Colaboradores:
- [FEDERICOMB](https://github.com/FEDERICOMB96)
- [metita](https://github.com/metita)
- [Mía](https://github.com/Mia2904)
- [Totopizza](https://github.com/oaus)
- [Roccoxx](https://github.com/Roccoxx)
- [r0ma](https://github.com/francoromaniello)

## Requerimientos
- [AmxModX](https://github.com/alliedmodders/amxmodx) >= 1.9.0.5263
- [ReHLDS](https://github.com/dreamstalker/rehlds) >= 3.9.0.752-dev
- [ReGameDLL](https://github.com/s1lentq/ReGameDLL_CS) >= 5.20.0.516-dev
- [ReAPI](https://github.com/s1lentq/reapi) >= 5.19.0.217
- [ReSemiclip](https://github.com/rehlds/resemiclip) >= 2.3.9 (OPCIONAL)

## Características
#### JSON amigable:
* Todas las configuraciones se hacen mediante el archivo JSON. No existe límite de modos que puedan ser cargados y cada modo puede tener configuraciones diferentes, cvars, plugins y resemiclip.
***
#### Comando de Admin (amx_multimod):
* Poder administrar los modos cargados, ya sea activandolos o desactivandolos.
* Poder seleccionar un modo y un mapa del modo y cambiarlo.
* Poder seleccionar un modo y enviar una votación de mapas con mapas seleccionados de ese modo.
* Poder iniciar una votación de modos.
* Admin en cuestión, al desconectarse su votación será cancelada.
***
#### Nominaciones:
* Es posible nominar modos disponibles para que puedan aparecer en el próximo votemod.
* Luego del votemod, es posible nominar mapas para que puedan aparecer en el próximo votemap.
* Las nominaciones son 1 (una) por jugador dentro del servidor y pueden ser removidas por el jugador mismo. Al desconectarse se eliminará su nominación.
***
#### Votemod:
* Antes de finalizar cada mapa, se envía un votemod automático con modos al azar (o modos nominados) a todo el servidor para elegir el próximo. En caso de empate con 2 o más opciones, se hará otra votación continuada para desempatar.
* Cada voto de cada jugador será removido si el mismo se desconecta durante la votación.
***
#### Votemap:
* Seguida del votemod, se enviará un votemap automático con mapas al azar (o mapas nominados) a todo el servidor para elegir el próximo. En caso de empate con 2 o más opciones, se hará otra votación continuada para desempatar.
* Cada voto de cada jugador será removido si el mismo se desconecta durante la votación.
***
#### Rock The Vote:
* Se podrá hacer uso del famoso "rtv" para solicitar una votación por los usuarios. La misma podrá contener modos/mapas nominados por los jugadores o seleccionados al azar.
***
#### Modos y mapas recientemente jugados:
* Se podrá establecer una cantidad de modos y mapas recientemente jugados para que no vuelvan a salir en la votación durante un tiempo elegido por la persona que lo configure.
***
#### Compatiblidad con ReSemiclip
* Se creó la compatibilidad para que la persona que lo configure pueda decidir diferentes configuraciones para cada modo en particular.
****
## Compilación
Descargar todos los archivos necesarios a una carpeta cualquiera, abrir el archivo multimod_manager.sma y compilar desde ahí. No es necesario copiar los archivos .inc a la carpeta includes de donde tengas el compilador.
