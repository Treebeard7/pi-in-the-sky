# Pi in the Sky Protocol

Byte - Message

0 - synchronization message

- Send a zero to indicate message start

1 - encoding of message type [command] (follow on payload size)

- 1: Throttle (1 bytes)
- 2: Aileron (1 bytes)
- 3: Elevator (1 bytes)
- 4: Rudder (1 bytes)
- 5: Bomb Drop (1 bytes)
- 6: Flight Mode (1 bytes)
- 7: Flight Termination(0 bytes)
- 90: Hearbeat (0 bytes)

2:n-1 - type value [command]
n(last byte) - EOT

# Example Messages

Set aileron 50: { 0, 2, 0, 5 }
Toggle flight mode: { 0, 6 }

# Flight Modes:

- 0: Manual
- 1: stabilized
