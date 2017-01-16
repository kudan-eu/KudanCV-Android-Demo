# KudanCV - Android Demo
* * *
This repository contains an Android Studio project with example code for an application that uses the KudanCV standalone library to perform image-based and markerless tracking of objects viewed by the camera.

This code uses the `android.hardware.camera2` package and as such requires at least Android 5.0 to run. Similar results can be achieved using the deprecated `Camera` class if legacy support is required.

## Working with Source
___

- Clone the project.
- Download the latest KudanCV release.
- Place the `.so` library files into their appropriate, newly-created ABI directory at:  
  
	`app/libs/KudanCV/bin/<abi>/libKudanCV.so`


- Place the header files into a newly-created directory at:


	`app/libs/KudanCV/include`

- Copy the development license key from [Kudan wiki](https://wiki.kudan.eu/Development_License_Keys) and add it to the `AndroidManifest.xml`to the value of `eu.kudan.ar.API_KEY`

		<meta-data
            android:name="eu.kudan.ar.API_KEY"
            android:value="LICENSE_KEY_HERE">
        </meta-data>


- Build and run the project.
