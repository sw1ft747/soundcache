# Soundcache Autosave
This is a [SvenMod](https://github.com/sw1ft747/svenmod) plugin that automatically saves and uses soundcache (separately for each server) that you download from a server during joining. At this time, the plugin will use saved soundcache that will skip the process of downloading it, thus you can enter into the game faster.

# Console Variables
ConVar | Default Value | Type | Description
--- | --- | --- | ---
soundcache_autosave | 1 | bool | Store and use saved soundcache separately for each server
soundcache_dont_flush | 0 | bool | Don't flush downloaded soundcache
soundcache_ignore | 0 | bool | Don't download/save/use soundcache (there will be almost no sounds on a server)

# Config
When the plugin will load, it will execute **.cfg** file from this folder: `Sven Co-op/svencoop/soundcache.cfg`

You can use this file to store values of cvars of the plugin, for example:
```
soundcache_autosave 0
soundcache_dont_flush 0
soundcache_ignore 1
```

# Common Issue
Sometimes server can update its sounds and soundcache will change, in this case you will hear weird untypical sounds.

To solve it, go to this folder: `Sven Co-op/svencoop_downloads/maps/soundcache/`.

Find the folder with the name that starts with IP of the server, open it. Then find the `*.txt` file with the name of map on which you have irregular soundcache, delete this file.
