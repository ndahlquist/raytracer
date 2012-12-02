
package edu.stanford.nicd.raytracer;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.RadioButton;
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
		((RadioButton) findViewById(R.id.radioOriginal)).toggle();
		((CheckBox) findViewById(R.id.checkBoxMapping)).toggle();

		mImageView = (ImageView) findViewById(R.id.imageView1);
		
		recomputeImages();
	}
	
	@Override
    public void onPause(){
        super.onPause();
        raytraceThread.done=true;
    }


	private void recomputeImages() {
		
		raytraceThread = new BitmapWorkerTask(mImageView);
		raytraceThread.execute(0);
		
		/*if(mImage == null) {
			final BitmapFactory.Options options = new BitmapFactory.Options();
			options.inScaled = false;	// No pre-scaling
			mImage = BitmapFactory.decodeResource(getResources(), R.drawable.tahoe, options);
		}
		RayTrace(mImage);
		ImageView mImageView = (ImageView) findViewById(R.id.imageView1);
		mImageView.setImageBitmap(mImage);*/
	}

	class BitmapWorkerTask extends AsyncTask<Integer, Void, Bitmap> {
	    private final WeakReference<ImageView> imageViewReference;
	    private int data = 0;
	    private boolean done = false;

	    public BitmapWorkerTask(ImageView imageView) {
	        // Use a WeakReference to ensure the ImageView can be garbage collected
	        imageViewReference = new WeakReference<ImageView>(imageView);
	    }

	    // Decode image in background.
	    @Override
	    protected Bitmap doInBackground(Integer... params) {
	        data = params[0];

			if(mImage == null) {
				final BitmapFactory.Options options = new BitmapFactory.Options();
				options.inScaled = false;	// No pre-scaling
				mImage = BitmapFactory.decodeResource(getResources(), R.drawable.tahoe, options);
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
	    	
	    }
	    
	    // Once complete, see if ImageView is still around and set bitmap.
	    @Override
	    protected void onPostExecute(Bitmap bitmap) {
	        if (imageViewReference != null && bitmap != null) {
	            final ImageView imageView = imageViewReference.get();
	            if (imageView != null) {
	                imageView.setImageBitmap(bitmap);
	            }
	        }
	    }
	}
	
	/* load our native library */
	static {
		System.loadLibrary("plasma");
	}

	private static native void RayTrace(Bitmap output, int frame);
}
