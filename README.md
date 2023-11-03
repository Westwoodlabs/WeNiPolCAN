# WeNiPol CAN

## Work in progress

## Notes

### Startup
On startup the ESP sends a baked-in Westwoodlabs logo to the WeNiPol.

### WiFi
This project uses WiFi-Manager to connect to a WiFi network. 
If the ESP cannot connect to a network, it will create its own network with the SSID `WeNiPol setup`.

When connected to an existing WiFi network, the ESP will announce itself via mDNS as `WeNiPol`
(`wenipol.local`). You probably should only use the mDNS lookup to determine the IP address
(e.g. via `ping`) and then use the IP for `curl` requests, as this mDNS is pretty slow.

### Uploading GIFs

- GIFs MUST be 48x48 pixels.
- The red-channel controls the red-LEDs, the green-channel controls the yellow-LEDs. \
  The LED will be on if the value is 0xF0 or greater.
- If all channels (R,G and B) are 0xF0 or greater, the LED won't be lit, \
  as "white" is interpreted as a background color.
- GIFs MAY be animated and contain any number of frames. The animation timing will be applied, \
  but the animation will be looped indefinitely always.
- To upload a GIF use the following endpoint:
    ```shell
    $> curl -F gif=@$filename http://$IP/gif
    ```
  If you want the GIF to be displayed imediately, add `?show` to the URL.
 - To show an existing GIF use:
    ```shell
    $> curl -X POST 'http://$IP/gif/$FILENAME/:show'
    ```
 - To list all GIFs use:
    ```shell
    $> curl 'http://$IP/gif'
    ```
 - To delete a GIF use:
    ```shell
    $> curl -X DELETE 'http://$IP/gif/$FILENAME'
    ```
 - To set the brightness use the following endpoint:
     ```shell
     $> curl -X POST 'http://$IP/brightness?brightness=$VALUE'
     ```
   The brightness value MUST be between 0 and 2400. The LEDs will only be visibly lit \
   from values about 130 and up.
 - You can also reboot the ESP, if anything goes wrong:
     ```shell
     $> curl -X POST 'http://$IP/reboot'
     ```