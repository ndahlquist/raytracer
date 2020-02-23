Archived and not maintained! This was an old school project- you probably shouldn't do raytracing on CPU anyway :)


###

This is a threaded CPU ray tracer written in native C++. On a multi-core device, it should run at close to real time (About 10 interlaced frames per second on my 2012 Nexus 7).
- Acceleration structures, interlacing, native implementation and multi-threading make this a very fast ray tracer. You can interact with the spheres during the render: something that is not possible with any other ray tracer currently on the Android Play Store.
- Environment mapping and recursive reflections.

You can download the compiled APK from the [Google Play Store] (https://play.google.com/store/apps/details?id=edu.stanford.nicd.raytracer&hl=en).

![Screenshot](https://lh5.ggpht.com/5fFmjuxzPZ2c7W2L_IeQLlt6Sd0em1-oINXJGgCMq4GEf4TG-5UtZUFr4bqVfiDO3ZLm=h900-rw)

Building:
Make sure the Android SDK and NDK are both installed and added to the PATH.
Run `./gradlew installDebug`
