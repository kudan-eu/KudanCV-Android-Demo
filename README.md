# KudanCV - Android Demo
* * *
This repository contains an Android Studio project with example code for an application that uses the KudanCV standalone library to perform image-based and markerless tracking of objects viewed by the camera.

This code uses the `android.hardware.camera2` package and as such requires at least Android 5.0 to run. Similar results can be achieved using the deprecated `Camera` class if legacy support is required.

## Working with Source
___

- Clone the project.
- Download the latest KudanCV release.
- Place the `.so` library files into their appropriate, newly-created ABI directory at:  
  
	`app/libs/KudanCV/bin/<abi>/libKudan.so`


- Place the header files into a newly-created directory at:


	`app/libs/KudanCV/include`


- Build and run the project.
