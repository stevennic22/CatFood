# Cat Food
### Feed your cat with a robot because cats can't love you anyway.
> A simple cat feeder script for [Arduino Yún](https://store.arduino.cc/usa/arduino-yun)/[Yún Mini](https://store.arduino.cc/usa/arduino-yun-mini)/[Yún Shield](https://store.arduino.cc/usa/arduino-yun-shield) boards.

#### Features

- Keep track of whether cat was feed during 'active' times
- Automatically feed cat if not feed within time bounds
- Connect to wifi and get 'auto' status
  - 'auto' status check persists through wifi connection loss
- Fires HTTP request to a server that tracks feeding times (when feeding happens)
- Utilize Yún bridge to have status information available over wifi

#### Commands
 
| Command | Purpose |
| - | - |
| Feed | Spins motor to feed (regardless of time bounds) |
| Auto | Spins motor to feed (within time bounds and not more than once) |
| Toggle | Turns on/off 'auto' status |

##### Language(s)

- Arduino (C/C++)

##### Depends

| External Libraries | License |
| --------- | --------- |
| [ArduinoJson](https://bblanchon.github.io/ArduinoJson/) | [MIT License](https://github.com/bblanchon/ArduinoJson/blob/master/LICENSE.md) |

##### Instructions

- Load into Arduino IDE of your preference
- Select Arduino board
- Compile/Upload
- Feed cat

### License
[License](LICENSE.md)