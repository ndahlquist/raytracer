package edu.stanford.nicd.raytracer;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Display;
import android.view.MotionEvent;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Switch;
import android.widget.TextView;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;

public class MainActivity extends Activity {
	
	private Bitmap mImage;
	private Bitmap mLightProbe;
	private Bitmap mBackground;
	private LinearLayout mLinearLayout;
	private RaytraceTask raytraceThread;
	int animationSpeed;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.main);
		mLinearLayout = (LinearLayout) findViewById(R.id.background);
		mLinearLayout.setOnTouchListener(new View.OnTouchListener() {
			public boolean onTouch(View v, MotionEvent ev) {
			    raytraceThread.MultiTouch = ev;
				return true;
			}
	    });
		
		Switch switchSampling = (Switch) findViewById(R.id.switchSampling);
		switchSampling.setChecked(true);
		switchSampling.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				ToggleAdaptiveSampling(isChecked);
				raytraceThread.ClearStats = true;
			}
		});
		
		Switch switchInterlacing = (Switch) findViewById(R.id.switchInterlacing);
		switchInterlacing.setChecked(true);
		switchInterlacing.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				if(isChecked)
					SetInterlacing(2);
				else
					SetInterlacing(1);
				raytraceThread.ClearStats = true;
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
		seekBarSpeed.setProgress(8);
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
	    private boolean ClearStats = false;
		private long startTime;
		private long numRays;
		private int numFrames;
		private MotionEvent MultiTouch;

	    public RaytraceTask() {
	    	ClearStats();
	    }

	    public void ClearStats() {
    		((TextView) findViewById(R.id.RaysPerSecond)).setText("--- x10^6 Viewing Rays/Second");
    		((TextView) findViewById(R.id.FramesPerSecond)).setText("--- Frames/Second");
    		ResetStats();
			ClearStats = false;
	    }
	    
	    public void ResetStats() {
			startTime = System.currentTimeMillis();
			numRays = 0;
			numFrames = 0;
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
			if(mLightProbe == null) {
				mLightProbe = BitmapFactory.decodeResource(getResources(), R.drawable.light_probe);
				mLightProbe.setHasAlpha(false);
			}
			PassLightProbe(mLightProbe);
			if(mBackground == null) {
				mBackground = BitmapFactory.decodeResource(getResources(), R.drawable.background);
				mBackground.setHasAlpha(false);
			}
			PassBackground(mBackground);
			long lastUpdateTime = System.currentTimeMillis();
			while(!terminateThread) {
				if(MultiTouch != null) {
					if(MultiTouch.getAction() == MotionEvent.ACTION_UP)
						MultiTouch = null;
					else for(int p = 0; p < MultiTouch.getPointerCount(); p++)
						TouchEvent(MultiTouch.getX(p), MultiTouch.getY(p));
				}
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
	        final float secondsElapsed = (System.currentTimeMillis() - startTime) / 1000.0f;
			if(ClearStats)
				ClearStats();
			else if(secondsElapsed > 1.0f) {
	        	float RaysPerSecond = numRays / secondsElapsed;
	        	float FramesPerSecond = numFrames / secondsElapsed;
	        	((TextView) findViewById(R.id.RaysPerSecond)).setText(String.format("%.2f", RaysPerSecond / 1000000) + "x10^6 Viewing Rays/Second");
	        	((TextView) findViewById(R.id.FramesPerSecond)).setText(String.format("%.2f", FramesPerSecond) + " Frames/Second");
	        	ResetStats();
	        }
	    }
	}
	
	/* load our native library */
	static {
		System.loadLibrary("plasma");
	}

	private static native void Initialize(Bitmap input);
	private static native void PassLightProbe(Bitmap lightProbe);
	private static native void PassBackground(Bitmap background);
	private static native int RayTrace(Bitmap output, long timeElapsed);
	private static native void ToggleAdaptiveSampling(boolean enabled);
	private static native void SetInterlacing(int value);
	private static native void TouchEvent(float x, float y);
	
}
