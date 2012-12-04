package edu.stanford.nicd.raytracer;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Display;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;

public class MainActivity extends Activity {
	
	private Bitmap mImage;
	private LinearLayout mLinearLayout;
	private RaytraceTask raytraceThread;
	int animationSpeed;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.main);
		mLinearLayout = (LinearLayout) findViewById(R.id.background);
		
		CheckBox checkBoxSampling = (CheckBox) findViewById(R.id.checkBoxSampling);
		checkBoxSampling.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				ToggleAdaptiveSampling(isChecked);
				raytraceThread.startTime = System.currentTimeMillis();
				raytraceThread.numRays = 0;
				raytraceThread.numFrames = 0;
			}
		});
		
		SeekBar seekBarSpeed = (SeekBar) findViewById(R.id.seekBarSpeed);
		seekBarSpeed.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {       
				animationSpeed = progress;
			}
			public void onStartTrackingTouch(SeekBar seekBar) {}
			public void onStopTrackingTouch(SeekBar seekBar) {}  
		});
	}
	
	@Override
    public void onResume(){
        super.onResume();
        raytraceThread = new RaytraceTask();
		raytraceThread.execute(0);
    }
	
	@Override
    public void onPause(){
        super.onPause();
        raytraceThread.terminateThread=true;
    }

	class RaytraceTask extends AsyncTask<Integer, Void, Bitmap> {
	    private boolean terminateThread = false;
		private long startTime = 0;
		private long numRays = 0;
		private int numFrames = 0;
		
	    public RaytraceTask() {
	        startTime = System.currentTimeMillis();
	    }

	    // Do raytracing in background
	    @Override
	    protected Bitmap doInBackground(Integer... params) {
			if(mImage == null) {
				Display display = getWindowManager().getDefaultDisplay();
				Point size = new Point();
				display.getSize(size);
				mImage = Bitmap.createBitmap(size.x, size.y, Bitmap.Config.ARGB_8888);
				mImage.setDensity(Bitmap.DENSITY_NONE);
				mImage.setHasAlpha(false);
			}
			Initialize(mImage);
			long lastUpdateTime = System.currentTimeMillis();
			while(!terminateThread) {
				long timeElapsed = System.currentTimeMillis() - lastUpdateTime;
				lastUpdateTime = System.currentTimeMillis();
				numRays += RayTrace(mImage, animationSpeed * timeElapsed);
				publishProgress();
			}
	        return mImage;
	    }

	    @SuppressWarnings("deprecation")
		@Override
	    protected void onProgressUpdate(Void... v) {
	        if(mImage != null) {
	            Drawable d = new BitmapDrawable(getResources(), mImage);
	            mLinearLayout.setBackgroundDrawable(d);
	        }
	        numFrames++;
    		float RaysPerSecond = numRays / ((System.currentTimeMillis() - startTime) / 1000.0f);
    		float FramesPerSecond = numFrames / ((System.currentTimeMillis() - startTime) / 1000.0f);
    		((TextView) findViewById(R.id.RaysPerSecond)).setText(String.format("%.2f", RaysPerSecond  / 1000000) + "x10^6 Primary Rays/Second");
    		((TextView) findViewById(R.id.FramesPerSecond)).setText(String.format("%.2f", FramesPerSecond) + " Frames/Second");
	    }
	}
	
	/* load our native library */
	static {
		System.loadLibrary("plasma");
	}

	private static native void Initialize(Bitmap input);
	private static native int RayTrace(Bitmap output, long timeElapsed);
	private static native void ToggleAdaptiveSampling(boolean enabled);
	
}
