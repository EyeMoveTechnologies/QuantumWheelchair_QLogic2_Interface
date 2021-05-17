import serial_arduino
import keyboard

# speed, dir
port = serial_arduino._make_serial_connection()
deflections = [255//2, 255//2] # 255//2 = 127 = neuttral for no movement
delta = 5

def send_down():
    packet = serial_arduino._format_packet(deflections)
    serial_arduino._send_packet_to_arduino(port, packet)
    serial_arduino._wait_for_acknowledge(port, 1, deflections)

def get_input(e):
    pressed = False
    if keyboard.is_pressed('up arrow'):
        pressed = True
        deflections[0] += delta
    if keyboard.is_pressed('down arrow'):
        pressed = True
        deflections[0] -= delta
    if keyboard.is_pressed('left arrow'):
        pressed = True
        deflections[1] -= delta
    if keyboard.is_pressed('right arrow'):
        pressed = True
        deflections[1] += delta

    deflections[0] = min(max(deflections[0], 0), 255)
    deflections[1] = min(max(deflections[1], 0), 255)

    if pressed:
        send_down()

def main():
    keyboard.on_press_key('up arrow', callback=get_input)
    keyboard.on_press_key('down arrow', callback=get_input)
    keyboard.on_press_key('left arrow', callback=get_input)
    keyboard.on_press_key('right arrow', callback=get_input)

    send_down()

    while(True):
        continue

if __name__ == "__main__":
    main()
