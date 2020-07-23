# medimaging
medical imaging analysis using dicom server

### Portal images
![Alt text](images/imagelist_portal.png?raw=true "Image list portal")
![Alt text](images/imageDetails_portal.png?raw=true "Image details(One image dcm = multiple png files")


# Installation instructions
The software uses nodejs for running the backend REST API and Orthanc DICOM server to interface over REST API with dicom images.
Hence you would require nodejs version > v10.16.3. Current version used to test is: v10.16.3

### Steps:
    - git clone https://github.com/nilaysaha/medimaging.git
    #### Install the web viewer
    - cd medimaging/src/viewer
    - npm install (uses package.json to install dependencies and create node_modules directory)
    - npm start (to start the web application for viewing the images)

    ### Create binary for orthanc server
    - cd server/Orthanc-1.7.2
    - ./build_static.sh (This imports static files from remote repo and build a orthanc server in a newly created "Build" directory)
    - ./Build/Orthanc (to start the dicom complaint REST api server)


    - Compare the output with the sample output.
        - once run it will stop because there is a waiting time for setting of temperature (sort of stabilization time when the boiler waits till the temperature stabilizes and this decreases as the difference in target and current temperature decreases.)
    - Once the temperature set by boiler becomes closer to the target temperature the increments of valve opening decreases exponentially. Till a tolerance limit predefined in code is reached. Then it stops iteration and ends with sample output shown above
    - Please note: We are using the mqtt server from 'mqtt://test.mosquitto.org'. Hence no need to run a local mqtt for this purpose.
License
----
MIT
