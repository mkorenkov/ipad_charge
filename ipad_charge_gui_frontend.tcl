#!/usr/bin/wish

#Author: Rani Fayez Ahmad (Superlinux)
#Date:22 July 2013
#ranixlb@gmail.com
#Interface/Frontend for ipad_charge
wm title . {Linux Apple Device USB Charging}
wm geometry . 500x100
wm resizable . 0 0
if { [ catch  { exec  lsusb -d 05ac:  } lsusb_outcome options ] }  {
tk_messageBox -detail "NO Apple Device Found!\n Please plug in the device to charge the battery"
exit
} 

set splits [ split $lsusb_outcome ]

set busnum [ lindex $splits 1 ]
set devnum [string trim [ lindex $splits 3 ] ":" ]

set env_var "/usr/bin/env BUSNUM=$busnum DEVNUM=$devnum"

proc turn_on_charging_mode {} {
global env_var

set cwd [pwd]
set charger_command "exec $env_var $cwd/ipad_charge"
if { [ catch  $charger_command  outcome options ] }  {
tk_messageBox -detail " Please install ipad_charge and run this script in the same folder of ipad_charge \n $outcome"
exit
} 
.status_label configure -text "Apple device USB charger is currently ON"
}

proc turn_off_charging_mode {} {
global env_var
set cwd [pwd]
set charger_command "exec $env_var $cwd/ipad_charge -0"
if { [ catch  $charger_command outcome options ] }  {
tk_messageBox -detail " Please install ipad_charge and run this script in the same folder of ipad_charge\n $outcome "
exit
} 
.status_label configure -text "Apple device USB charger is currently OFF"
}

button .turnoncharger -text {ON} -command turn_on_charging_mode
button .turnoffcharger -text {OFF} -command turn_off_charging_mode
label .status_label 
place .turnoncharger -x 150 -y 20
place  .turnoffcharger -x 250 -y 20
place .status_label -x 10 -y 50
