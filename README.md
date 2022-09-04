# Soundcache Autosave
This is a [SvenMod](https://github.com/sw1ft747/svenmod) plugin that automatically saves and uses soundcache that you download from a server during connection. During connection, the plugin will use saved soundcache that will skip the process of downloading it, thus you can enter into the game faster.

# Console Variables
ConVar | Default Value | Type | Description
--- | --- | --- | ---
autosave_soundcache | 1 | bool | Store and use saved soundcache

# Common Issue
Sometimes server can update its sounds and soundcache will change, in this case you will hear weird untypical sounds.

To solve it, go to this folder: `Sven Co-op/svencoop_downloads/maps/soundcache/`.

Find the folder with the name that starts with IP of the server, open it. Then find the `*.txt` file with the name of map on which you have irregular soundcache.
