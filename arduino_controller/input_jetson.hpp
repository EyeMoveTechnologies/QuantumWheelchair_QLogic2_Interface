#ifndef __INPUT_JETSON_HPP__
#define __INPUT_JETSON_HPP__

// Packet Indexes
#define SPEED 0U
#define DIRECTION 1U
#define PACKET_LENGTH (1 + 1 + 1 + 1) // 2 Delimiters, 2 * 1 bytes

void setup_jetson_serial() {
  Serial.begin(9600);
}

void clean_garbage_packet() {
  // read unreliable serial data until we either find the next packet or run out of data
  while(Serial.available() > 0) {
    // Unless next byte is start of a good packet (let the next call handle it), read & ignore it
    if (Serial.peek() == '[') {
      break;
    } else {
      Serial.read();
    }
  }
}

bool parse_good_packet(int deflections[2])   {
   // parse through & format data from good packet
  if (Serial.available() >= PACKET_LENGTH) {

    Serial.read(); // Read through open delimiter
    deflections[0] = Serial.read();
    deflections[1] = Serial.read();

    if (Serial.read() == ']') {
      return true;
    } else { // Invalid close delimiter
      clean_garbage_packet();
      return false;
    }

  } else {
    return false;
  }

}

bool get_deflections_from_jetson(int deflections[2]) {
  if (Serial.available() > 0) { // If data available

    if (Serial.peek() == '[') { // If open delimiter is correct
      return parse_good_packet(deflections);

    } else { // If not the open delimiter, we do not know @ what point in the packet we are, incomplete data
      clean_garbage_packet();
      return false;
    }

  } else {
    return false;
  }
}

#endif // __INPUT_JETSON_HPP__
