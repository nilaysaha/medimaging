# medimaging
medical imaging analysis using dicom server

![Alt text](images/exam_wattx.png?raw=true "Title")




### Sample help
```sh
node set_temp.js  --help
Usage: set_temp [options]

Options:
  -t, --targetTemp <number>  Set the target temperature of the room (default: 23.5)
  -i, --initTemp <number>    Set the initial temperature of the room (default: 18)
  -h, --help                 display help for command
```




# Installation instructions
The software uses nodejs. Hence you would require nodejs version > v10.16.3. Current version used to test is: v10.16.3

### Steps:
    - git clone https://github.com/nilaysaha/temperature_controller.git
    - cd temperature_controller
    - npm install (uses package.json to install dependencies and create node_modules directory)
    - To see help: node set_temp.js --help
    - Run the code: node set_temp.js
    - Compare the output with the sample output.
        - once run it will stop because there is a waiting time for setting of temperature (sort of stabilization time when the boiler waits till the temperature stabilizes and this decreases as the difference in target and current temperature decreases.)
    - Once the temperature set by boiler becomes closer to the target temperature the increments of valve opening decreases exponentially. Till a tolerance limit predefined in code is reached. Then it stops iteration and ends with sample output shown above
    - Please note: We are using the mqtt server from 'mqtt://test.mosquitto.org'. Hence no need to run a local mqtt for this purpose.
License
----
MIT
