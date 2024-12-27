import can
import time

def flip_significant_byte(value):
    msb = (value >> 8) & 0xFF
    lsb = value & 0xFF
    return (lsb << 8) | msb

# Create a CAN bus interface object
bus = can.interface.Bus(channel='can0', bustype='socketcan')

start_time = time.time()
start_time2 = time.time()
FRAME_START = 1
FRAME_END = 1
frame_nummer = FRAME_START

FADE_BRIGHTNESS = False
BRIGHTNES_MAX = 2400
BRIGHTNES_MIN = 200
brightness = 2400 # 100%
brightness = 2000 # 50%
count_up = False

try:
    while True:

        current_time = time.time()
        elapsed_time = current_time - start_time
        elapsed_time2 = current_time - start_time2
        
        
        power = True

        data=[0x00, # Frame number
            0x00, # 00 = on, 0f = off
            0x00, # Brightness LSB # 6009 = 100% (2400) # d002 = 30% (720) # c800 = 5% (200) 
            0x00, # Brightness MSB 
            0xee,
            0xff,
            0x00,
            0x00]
        arbitration_id=0x1fc01fdf

        data[0] = frame_nummer
        data[1] = 0x00 if power else 0x0f
        data[2] = brightness & 0xFF
        data[3] = (brightness >> 8) & 0xFF

        #print(''.join(format(byte, '02X') for byte in data))

        # Define a CAN message
        message = can.Message(arbitration_id=arbitration_id, data=data)
        
        # Send the CAN message
        bus.send(message)
        #print("Message sent:", message)

        if elapsed_time >= 0.5:
            frame_nummer+=1
            start_time = current_time
            print("frame_nummer: ", frame_nummer)

        if frame_nummer > FRAME_END:
            frame_nummer = FRAME_START

        if FADE_BRIGHTNESS:
            if elapsed_time2 >= 0.1:
                if count_up:
                    brightness += 200
                else:
                    brightness -= 200
                start_time2 = current_time
                print("brightness: ", brightness)

            if brightness <= BRIGHTNES_MIN:
                brightness = BRIGHTNES_MIN
                count_up = True
            elif brightness >= BRIGHTNES_MAX:
                brightness = BRIGHTNES_MAX
                count_up = False
            
            
        
        time.sleep(0.001)  # Add a delay between message sends
        
except KeyboardInterrupt:
    print("\nLoop interrupted by user.")


