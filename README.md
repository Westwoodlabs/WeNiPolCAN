# WeNiPol CAN

## Work in progress

## Notes

### Startup
On startup the ESP sends a baked-in Westwoodlabs logo to the WeNiPol.

### WiFi
This project uses WiFi-Manager to connect to a WiFi network. 
If the ESP cannot connect to a network, it will create its own network with the SSID `WeNiPol setup`.

### Uploading GIFs

- GIFs MUST be 48x48 pixels.
- The red-channel controls the red-LEDs, the green-channel controls the yellow-LEDs. \
  The LED will be on if the value is 0xF0 or greater.
- If all channels (R,G and B) are 0xF0 or greater, the LED won't be lit, \
  as "white" is interpreted as a background color.
- GIFs MAY be animated and contain up to 63 frames. The animation timing will be applied, \
  but the animation will be looped indefinitely always.
- To upload a GIF use the following endpoint:
    ```shell
    $> curl -F gif=@$filename http://$IP/gif
    ```
 - To set the brightness use the following endpoint:
     ```shell
     $> curl -X POST 'http://$IP/brightness?brightness=$VALUE'
     ```
   The brightness value MUST be between 0 and 2400. The LEDs will only be visibly lit \
   from values about 130 and up.