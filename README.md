# pientertain
Hue Entertain from Raspberry Pi Camera

### Motivation
Use a Raspberry Pi to film your TV or monitor and let it control your lights using the Hue Entertain API.

Pientertain has a simple REST-like interface to control camera parameters and to define the area, where the display is located
in the camera image. An example Android app to control pientertain can be found at https://github.com/bulletsk/pientertaincontrol.

This project is for tinkering, proof-of-concept and learning. If you are looking for a complete solution to control your lights from different brands and types using various frame grabbers, check out Hyperion.ng here: https://github.com/hyperion-project/hyperion.ng.

### Installation
Pientertain uses features of Qt 5.12 which doesnt come with raspbian yet unfortunately. To build my version of Qt 5.12 for the
Pi I used these guides:

- https://www.raspberrypi.org/forums/viewtopic.php?t=204778
- https://mechatronicsblog.com/cross-compile-and-deploy-qt-5-12-for-raspberry-pi

To control the camera, pientertain uses RaspiCam, which is very easy to compile and install on the pi. However, I could not get 
a cross-compiled version to run on the Pi, and a natively compiled one didnt work with the cross-compiler.
As a quick remedy I created a slim c-only interface to RaspiCam, which can be found in the folder raspicaminterface.

So here is the step-by-step guide:
- cross-compile Qt5, install on Pi
- compile and install RaspiCam natively on the Pi
- cross-compile RaspiCam, install to cross-compilation sysroot
- compile and install raspicaminterface natively on the Pi
- cross-compile raspicaminterface, install to cross-compilation sysroot
- cross-compile pientertain and deploy to Pi

If somehow your cross-compiled RaspiCam works on the Pi, let me know how.

### Additional steps
In my usage scenario the Pi is dedicated to pientertain, so I made it autostart at boot time. Pientertain's API has a 
shutdown command, where it executes "sudo shutdown now" and shuts down the Pi. For this to work, shutdown must be set up
to work without a password.

### Configuration and API
All settings are saved using QSettings. The config file can usually be found at /home/username/.config/pientertain.conf or /home/username/.config/appkellner/pientertain.conf. Some settings like hue bridge ip, server port and image sensor width/height can only be set here.

To control everything else, use http requests to the server port (default 8999):

```
http://raspi.local:8999/status
reply:
{
    "bridge": "S/E hue bridge status message",
    "stream": "S/E hue entertain stream status message",
    "video": = "S/E raspi camera status message",
    "camerasettings" = { camera settings json object, see below }
}
(a leading S indicates that the message is status information, a leading E indicates an error)
On first use, the "bridge" message will read "link button not pressed" to 
indicate that you need to pair your Pi to your Hue Bridge.
```

```
http://raspi.local:8999/start
http://raspi.local:8999/stop
http://raspi.local:8999/shutdown
reply:
{}
Start/stop streaming light controls. Shutdown will exit pientertain and try to shutdown the Pi. 
```

```
http://raspi.local:8999/corners
reply:
[ { "x": 123, "y": 123 }, ... ]
An array of four corner points, indicating the location of the display in the camera image 
ordered top-left, top-right, bottom-left, bottom-right.
The corners can be set using a PUT request using the same array type.
```

```
http://raspi.local:8999/image
reply:
a JPEG compressed image of the current camera image
```

```
http://raspi.local:8999/camsettings
use a PUT request with a JSON object contain any or all of the following key-values:
{
    "brightness": 0 - 100,
    "saturation": -100 - 100,
    "contrast": -100 - 100,
    "whitebalance_r": 0 - 800,
    "whitebalance_b": 0 - 800,
    "shutter": 0 - 330000,    // in nanosecs
    "iso": 100 - 800,
    "area": 5 - 100,          // use image patches of area x area pixel 
                              // for color estimation
    "smooth": 0 - 100         // use mean of last 0-100 estimated colors for smooth 
                              // color changes
}
```

#### Requirements
- Raspberry Pi with camera module.
- Qt 5.12 or higher, https://www.qt.io
- RaspiCam 0.1.8 or higher, https://www.uco.es/investiga/grupos/ava/node/40
