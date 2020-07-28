# medimaging
medical imaging analysis using dicom server

### Overall process interaction diagram
![Alt text](images/deepc.ai.png?raw=true "Process interaction diagram")

### Portal images
![Alt text](images/imagelist_portal.png?raw=true "Image list portal")
![Alt text](images/imageDetails_portal.png?raw=true "Image details(One image dcm = multiple png files")


# Installation instructions
The software uses nodejs for running the backend REST API and Orthanc DICOM server to interface over REST API with dicom images.
Hence you would require nodejs version > v10.16.3. Current version used to test is: v10.16.3. For unit testing we need to have mocha installed.

### Steps:
    - git clone https://github.com/nilaysaha/medimaging.git

#### Create binary for orthanc server
    - cd server/Orthanc-1.7.2
    - ./build_static.sh (This imports static files from remote repo and build a orthanc server in a newly created "Build" directory)
    - nohup ./Build/Orthanc & (to start the dicom complaint REST api server and detach it from terminal)

### Run a cron job for processing incoming images.
    - node src/monitor_changes.js & (This runs a job every 10 seconds which checks the Orthanc server started above, for new images. If found it parses and converts images to png for frontend viewer)


#### Install the web viewer
    - cd medimaging/src/viewer
    - npm install (uses package.json to install dependencies and create node_modules directory)
    - npm start (to start the web application for viewing the images)


## Testing:
   - There is a unit test created to see if the Orthanc server is working. 
   - To run the unit test:
     - cd medimaging
     - mocha tst/test.js  (this tests the server is up and running, inputting a dicom image & deleting the dicom image)


License
----
MIT
