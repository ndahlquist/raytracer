This is a threaded CPU ray tracer written in native C++. On a multi-core device, it should run at close to real time (About 10 interlaced frames per second on my 2012 Nexus 7).
- Acceleration structures, interlacing, native implementation and multi-threading make this a very fast ray tracer. You can interact with the spheres during the render: something that is not possible with any other ray tracer currently on the Android Play Store.
- Environment mapping and recursive reflections.

You can download the compiled APK from the [Google Play Store] (https://play.google.com/store/apps/details?id=edu.stanford.nicd.raytracer&hl=en).

![Screenshot](http://nicdahlquist.com/raytracer/img/crop.jpg)

Building:
Make sure the Android SDK and NDK are both installed and added to the PATH.
Run `./gradlew installDebug`
