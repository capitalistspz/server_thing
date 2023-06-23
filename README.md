# Server Thing
It is a thing for downloading the Minecraft server jars based on data from Mojang's [version manifest](https://piston-meta.mojang.com/mc/game/version_manifest_v2.json)

### Requirements
#### libcpr (I am too lazy to use curl)
- `dnf install cpr`
- https://github.com/libcpr/cpr


#### simdjson (trillions must parse)
- `dnf install simdjson`
- https://github.com/simdjson/simdjson

### Usage
It can be run without any arguments, and the version selected interactively:

`server_thing`

with the resulting file going to `./versions/<game_version>/server.jar`

Alternatively, it can be run with arguments, e.g.

`server_thing --version 1.20 --path ~/my_adventure_server/server.jar` - downloads the 1.20 server jar to the path `~/my_adventure_server/server.jar`

`server_thing --version 1.15` - downloads the 1.15 server jar to `./versions/1.15/server.jar`

`server_thing --path ./my_folder/file` downloads a server jar (interactively selected) to the path `./my_folder/file`
