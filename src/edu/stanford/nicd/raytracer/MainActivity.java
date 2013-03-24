package edu.stanford.nicd.raytracer;

import java.util.ArrayList;

import edu.stanford.nicd.raytracer.MainActivity.RaytraceTask.TouchTracker;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;

public class MainActivity extends Activity {
    
    private final String TAG = "raytracer";
    
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
		mLinearLayout.setOnTouchListener(new RaytracerTouchHandler());

		CompoundButton checkboxReflections = (CompoundButton) findViewById(R.id.checkboxReflections);
		checkboxReflections.setChecked(true);
		checkboxReflections.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				SetReflectionsEnabled(isChecked);
				raytraceThread.ClearStats = true;
			}
		});

		CompoundButton checkboxLightprobe = (CompoundButton) findViewById(R.id.checkboxLightprobe);
		checkboxLightprobe.setChecked(true);
		checkboxLightprobe.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				SetLightprobeEnabled(isChecked);
				raytraceThread.ClearStats = true;
			}
		});
		
		CompoundButton checkboxInterlacing = (CompoundButton) findViewById(R.id.checkboxInterlacing);
		checkboxInterlacing.setChecked(true);
		checkboxInterlacing.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				SetInterlacingEnabled(isChecked);
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
		if(mLightProbe == null) {
			mLightProbe = BitmapFactory.decodeResource(getResources(), R.drawable.light_probe);
			mLightProbe.setHasAlpha(false);
		}
		if(mBackground == null) {
			mBackground = BitmapFactory.decodeResource(getResources(), R.drawable.background);
			mBackground.setHasAlpha(false);
		}
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if(!hasFocus)
		    return;
		if(mLinearLayout.getWidth() == 0 || mLinearLayout.getHeight() == 0)
			return;
		if(mImage == null) {
			mImage = Bitmap.createBitmap(mLinearLayout.getWidth(), mLinearLayout.getHeight(), Bitmap.Config.ARGB_8888);
			mImage.setHasAlpha(false);
		}
		raytraceThread = new RaytraceTask();
		raytraceThread.execute();
	}

	@Override
	public void onPause(){
		super.onPause();
		raytraceThread = null;
	}
	
	class RaytracerTouchHandler implements View.OnTouchListener {
		public boolean onTouch(View v, MotionEvent ev) {
			final int action = ev.getActionMasked();
		    
		    if(action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN) {
			    final int actionIndex = ev.getActionIndex();
			    final int pointerID = ev.getPointerId(actionIndex);
			    final float x = ev.getX(actionIndex);
		        final float y = ev.getY(actionIndex);
		    	
		    	// Query the native code to see if this touch intersects a sphere.
		        final int sphereID = TraceTouch(x, y);
		        if(sphereID == -1)
		        	return false;
		        
		        // Add this touch to the list.
		        TouchTracker thisTouch = raytraceThread.new TouchTracker();
		        thisTouch.pointerID = pointerID;
		        thisTouch.sphereID = sphereID;
		        thisTouch.x = x;
		        thisTouch.y = y;
		        raytraceThread.touches.add(thisTouch);
		        return true;
		    }
		        
		    if(action == MotionEvent.ACTION_MOVE) {
		        // ACTION_MOVE events are batched, so we have to iterate over the pointers.
		        for(int i=0; i<ev.getPointerCount(); i++) {
		            // Find the matching touch.
			        final int pointerID = ev.getPointerId(i);
		            final int touchListIndex = getTouchListIndex(pointerID);
		        	if(touchListIndex == -1)
		        		continue;
		        	TouchTracker touch = raytraceThread.touches.get(touchListIndex);
                    // Update its position.
		        	touch.x = ev.getX(i);
		        	touch.y = ev.getY(i);
		    	}
		        return true;
		    }
		    
		    if(action == MotionEvent.ACTION_POINTER_UP) {
			    final int actionIndex = ev.getActionIndex();
			    final int pointerID = ev.getPointerId(actionIndex);
		        // Find the matching touch and remove it.
		        final int touchListIndex = getTouchListIndex(pointerID);
		    	if(touchListIndex == -1) 
		    		return false;
		    	raytraceThread.touches.remove(touchListIndex);
		        return true;
		    }
		        
		    if(action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_CANCEL) {
		        raytraceThread.touches.clear();
		        return true;
		    }
		    
		    Log.e(TAG, "Unhandled touch action " + action);
		    return false;
		}
		
		/* Returns the index of the elem in raytraceThread.touches
		   that matches pointerID, or -1 if no match is found. */
		private int getTouchListIndex(int pointerID) {
		    for(int i=0; i < raytraceThread.touches.size(); i++) {
				TouchTracker thisTouch = raytraceThread.touches.get(i);
				if(pointerID == thisTouch.pointerID)
					return i;
			}
			return -1;
		}
		
		/* Returns true if there is an elem in raytraceThread.touches
		   that matches sphereID. */
		/*private boolean checkSphereExists(int sphereID) {
		    for(int i=0; i < raytraceThread.touches.size(); i++) {
				TouchTracker thisTouch = raytraceThread.touches.get(i);
				if(sphereID == thisTouch.pointerID)
					return true;
			}
			return false;
		}*/
	}

	class RaytraceTask extends AsyncTask<Void, Integer, Bitmap> {
		private boolean ClearStats = false;
		private long startTime;
		private long numRays;
		private int numFrames;
		
		public class TouchTracker {
			int pointerID;
			int sphereID;
			float x;
			float y;
		}
		ArrayList<TouchTracker> touches = new ArrayList<TouchTracker> ();

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
		
		// Do ray tracing in background
		@Override
		protected Bitmap doInBackground(Void... params) {
			
			Initialize(mImage);
			PassLightProbe(mLightProbe);
			PassBackground(mBackground);
			long lastUpdateTime = System.currentTimeMillis();
			// Continue as long as owned by the MainActivity.
			while(MainActivity.this.raytraceThread == this) {
				for(int i=0; i < touches.size(); i++) {
					TouchTracker touch = touches.get(i);
					MoveTouch(touch.x, touch.y, touch.sphereID);
				}
				long timeElapsed = System.currentTimeMillis() - lastUpdateTime;
				lastUpdateTime = System.currentTimeMillis();
				int thisNumRays = RayTrace(mImage, animationSpeed * timeElapsed);
				publishProgress(thisNumRays);
			}
			return mImage;
		}

		@SuppressWarnings("deprecation")
		protected void onProgressUpdate(Integer... progress) {
			if(mImage != null) {
				Drawable d = new BitmapDrawable(getResources(), mImage);
				mLinearLayout.setBackgroundDrawable(d);
			}
			numRays += progress[0];
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

	// load our native library
	static {
		System.loadLibrary("raytracer");
	}

	private static native void Initialize(Bitmap input);
	private static native void PassLightProbe(Bitmap lightProbe);
	private static native void PassBackground(Bitmap background);
	private static native int RayTrace(Bitmap output, long timeElapsed);
	private static native void SetInterlacingEnabled(boolean enabled);
	private static native void SetReflectionsEnabled(boolean enabled);
	private static native void SetLightprobeEnabled(boolean enabled);
	private static native int TraceTouch(float x, float y);
	private static native void MoveTouch(float x, float y, int sphereIndex);

}
