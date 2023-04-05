# Manual
There are several steps to setup working CareLight. They will be described for ESP-WROOM-32 main board at it requires least effort.

## Hardware assembly
To avoid any soldering, make sure that you buy boards from manufacturer that provides already soldered modules. Often they come with goldpins just thrown into a bag with the module.

Use three female to female DuPont wires to make connections:
* GND on main board to ground of the LED board (G/GND/V-)
* VIN on main board to positive voltage of the LED board (VCC/+5V/V+)
* D12 on main board to data input of the LED board (DIN/IN/INPUT)

## Programming

1. Connect main board to the computer with mini USB cable to check if it is recognized as COM port.
  * Find in menu Start and open the Device Manager (Menadżer Urządzeń) 
  * Look under Ports (COM & LPT) for:
    !(./Documents/Media/DevMan.png)
    