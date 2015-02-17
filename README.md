This is a threaded CPU ray tracer written in native C++. On a multi-core device, it should run at close to real time (About 10 interlaced frames per second on my Nexus 7).
Features:
-Acceleration structures, interlacing, native implementation and multi-threading make this a very fast ray tracer. You can interact with the spheres during the render: something that is not possible with any other ray tracer currently on the Android Play Store.
-Environment mapping and recursive reflections.

You can get the compiled APK at https://play.google.com/store/apps/details?id=edu.stanford.nicd.raytracer&hl=en

Building:
Make sure the Android SDK and NDK are both installed and added to the PATH.
From the top level of the project:

Compile native source:
$ ndk-build

Configure ant (only neccesary to run once):
$ android update project -p . -t 1

Compile java, generate APK, and install:
$ ant debug install
