from PIL import Image


def pixels_to_payloads(px):
    # px is xy pixel data, 2bit (red, yellow) , 16 rows, 16 columns
    #
    # CAVEAT: the whole panel is rotated by 180 degrees, the following effectively describes
    #         the panel from bottom right to top left.
    #
    # 8 bytes per message (CAN max payload)
    # 2 messages (16 bytes) per "group"
    # there are four groups ("A", "B", "C", "D"; using indexes 0-3)
    # each group has 4 bits per color and row (two colors: first 4 bits are red, second 4 bits are yellow)
    # each bit is every fourth LED in the physical row
    payloads = []

    for group in range(3, -1, -1):
        payload = []
        for row in range(15, -1, -1):
            row_data = 0
            # red 4-bits in current group & row
            row_data |= (px[row * 16 + (3 * 4 + group)] & 0b10) << 6  # move 0bx0 to 0bx0000000
            row_data |= (px[row * 16 + (2 * 4 + group)] & 0b10) << 5  # move 0bx0 to 0b0x000000
            row_data |= (px[row * 16 + (1 * 4 + group)] & 0b10) << 4  # move 0bx0 to 0b00x00000
            row_data |= (px[row * 16 + (0 * 4 + group)] & 0b10) << 3  # move 0bx0 to 0b000x0000
            # yellow 4-bits in current group  & row
            row_data |= (px[row * 16 + (3 * 4 + group)] & 0b01) << 3  # move 0b0x to 0b0000x000
            row_data |= (px[row * 16 + (2 * 4 + group)] & 0b01) << 2  # move 0b0x to 0b00000x00
            row_data |= (px[row * 16 + (1 * 4 + group)] & 0b01) << 1  # move 0b0x to 0b000000x0
            row_data |= (px[row * 16 + (0 * 4 + group)] & 0b01) << 0  # move 0b0x to 0b0000000x
            payload.append(row_data)
            # split payload into two 8-byte messages, due to CAN restrictions
            if row == 8:
                payloads.append(payload)
                payload = []
        payloads.append(payload)
    return payloads


def get_pixels(image, x_segment, y_segment):
    px = []
    for y in range(y_segment * 16, (y_segment + 1) * 16):
        for x in range(x_segment * 16, (x_segment + 1) * 16):
            col = image.getpixel((x, y))
            px.append((col == 0x00) << 0)  # black to red
    return px


def main():
    image = Image.open("./WWL_Light_NoBG_48x48.png")
    address_common = 0x1F0E1FFF
    cmd_header = 0xB00000
    cmd_data = 0x900000
    cmd_trailer = 0x700000
    for y_segment in range(0, 3):
        for x_segment in range(0, 3):
            px = get_pixels(image, x_segment, y_segment)
            payloads = pixels_to_payloads(px)
            # incrementing panel-id is encoded in bits 15-18 (from the left, 0-indexed) and inverted
            addr = address_common | (0xF ^ (y_segment * 3 + x_segment + 1)) << 13
            # init frame 0
            #  send header
            print(hex(addr | cmd_header)[2:] + "#0000FFFDFFFF7FF7")
            #  send data (first line is different)
            print(hex(addr | cmd_data)[2:] + "#0001010101010101")
            for _ in range(7):
                print(hex(addr | cmd_data)[2:] + "#0101010101010101")
            #  send trailer
            print(hex(addr | cmd_trailer)[2:] + "#FF7F")
            # send frame 1
            #  send header
            print(hex(addr | cmd_header)[2:] + "#0100FFFDFFFF7FF7")
            #  send data
            for payload in payloads:
                print(hex(addr | cmd_data)[2:] + "#" + bytearray(payload).hex().zfill(16))
            #  send trailer
            print(hex(addr | cmd_trailer)[2:] + "#FF7F")


if __name__ == '__main__':
    main()
