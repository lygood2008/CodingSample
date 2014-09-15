Manual-Automation
=================

Manual Automation for Emulation Execution
---

Please note, you need to be 'root' for all the following processes

### Installation
1. Edit the 'AUTO_PATH' varable in the 'config.sh' to point to the installation directory.
   * The default directory is '/mnt/share/__automation__'
2. As root, run the './install.sh' script
   * This sets up a cron job for monitoring
   * Updates rsa keys needed to connect to filt\fmev machines
   * Updates the xen-br and generates a MAC address in the sample start scripts
   * Sets up PATH VARs for the scripts to be used properly

===

### Upgrade from 2.0+
1. Replace the scripts in your AUTO_PATH directory

===

### Upgrade from Legacy (from versions 1.5, 1.6, 1.7)
1. Edit the 'AUTO_PATH' varable in the 'config.sh' to point to the installation directory.
   * The default directory is '/mnt/share/__automation__'
2. As root, run the './upgrade.sh' script
   * This updates the crontab with the new PATH vars and new installation directory
   * Sets up PATH VARs for the scripts to be used properly
3. Overwrite all the files manually, replace the 'VM_STARTUP_3D*' files as needed
   * VM_STARTUP_3D is no longer required prefix

===

### Usage - Start
1. Modify OR create a new start script
2. Modify the config.sh 'VM_START' variable to reflect this new script
3. From that installed Directory, run './go_mon.sh' to start monitoring
4. Run './start_vm.sh' to start the VM (if you don't, go_mon will trigger a start anyway)
5. Your VM should have launch with hang monitoring

===

### Usage - Stop
1. run './stop_mon.sh' to stop the monitoring.
   * Your VM should still be running
   * The idle monitoring should stop
   * Your hang monitoring should be killed

===

### Seeing the Hang monitor 
1. In a terminal, type in 'screen -ls'
   * This should dump a list of screen process
2. Recall that screen session by 'screen -r [id of process]'

###### See example Below
```
[root@genxfsim-has310 __automation__]# screen -ls
There is a screen on:
        5960.has0 (Detached)
1 Socket in /var/run/screen/S-root.

[root@genxfsim-has310 __automation__]# screen -r 5960
```

===
### FAQ
Q: Can I run './start_vm.sh' twice?

A: Yes, it will kill the current VM that running


Q: Can I run './go_mon.sh' more that once in a row?

A: Yes, it will re-apply the monitoring flags and behavior should not change.


Q: Can I run './stop_mon.sh' more tha once in a row?

A: Yes, this will just remove any monitoring flags, if they exist.

