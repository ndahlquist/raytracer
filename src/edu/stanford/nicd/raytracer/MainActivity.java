
package edu.stanford.nicd.raytracer;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.TextView;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

public class MainActivity extends Activity {
	
	private Bitmap mImage;
	private ImageView mImageView;
	private BitmapWorkerTask raytraceThread;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.main);
		mImageView = (ImageView) findViewById(R.id.imageView1);
	}
	
	@Override
    public void onResume(){
        super.onResume();
        raytraceThread = new BitmapWorkerTask(mImageView);
		raytraceThread.execute(0);
    }
	
	@Override
    public void onPause(){
        super.onPause();
        raytraceThread.done=true;
    }

	class BitmapWorkerTask extends AsyncTask<Integer, Void, Bitmap> {
	    private final WeakReference<ImageView> imageViewReference;
	    private boolean done = false;
		private long lastMeterTime = 0;
		private int lastMeterFrame = 0;

	    public BitmapWorkerTask(ImageView imageView) {
	        // Use a WeakReference to ensure the ImageView can be garbage collected
	        imageViewReference = new WeakReference<ImageView>(imageView);
	        lastMeterTime = System.currentTimeMillis();
	    }

	    // Do raytracing in background
	    @Override
	    protected Bitmap doInBackground(Integer... params) {

			if(mImage == null) {
				final BitmapFactory.Options options = new BitmapFactory.Options();
				options.inScaled = false;	// No pre-scaling
				mImage = BitmapFactory.decodeResource(getResources(), R.drawable.background, options);
			}
			int frame = 0;
			while(!done) {
				RayTrace(mImage, frame++);
				publishProgress();
			}
	        return mImage;
	    }

	    @Override
	    protected void onProgressUpdate(Void... v) {
	        if (imageViewReference != null && mImage != null) {
	            final ImageView imageView = imageViewReference.get();
	            if (imageView != null) {
	                imageView.setImageBitmap(mImage);
	            }
	        }
	        lastMeterFrame++;
    		if(System.currentTimeMillis() - lastMeterTime >= 1000) {
    			float FramesPerSecond = lastMeterFrame / ((System.currentTimeMillis() - lastMeterTime) / 1000.0f);
    			((TextView) findViewById(R.id.FPS)).setText("FPS: " + String.format("%.2f", FramesPerSecond));
    			lastMeterTime = System.currentTimeMillis();
    			lastMeterFrame = 0;
    		}
	    }
	}
	
	/* load our native library */
	static {
		System.loadLibrary("plasma");
	}

	private static native void RayTrace(Bitmap output, int frame);
}
