package edu.stanford.nicd.raytracer;

import java.util.ArrayList;

import edu.stanford.nicd.raytracer.MainActivity.RaytraceTask.TouchTracker;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
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

	private Bitmap mImage;
	private Bitmap mLightProbe;
	private Bitmap mBackground;
	private LinearLayout mLinearLayout;
	private RaytraceTask raytraceThread;
	//private MotionEvent MultiTouch;
	int animationSpeed;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.main);

		mLinearLayout = (LinearLayout) findViewById(R.id.background);
		mLinearLayout.setOnTouchListener(new View.OnTouchListener() {
			public boolean onTouch(View v, MotionEvent ev) {
				final int action = ev.getAction();
			    switch (action & MotionEvent.ACTION_MASK) {
			    case MotionEvent.ACTION_DOWN: {
			        final float x = ev.getX(0);
			        final float y = ev.getY(0);
			        
			        int sphereID = TraceTouch(x, y);
			        if(sphereID == -1)
			        	break;
			        
			        TouchTracker thisTouch = raytraceThread.new TouchTracker();
			        thisTouch.pointerID = ev.getPointerId(0);
			        thisTouch.sphereID = sphereID;
			        thisTouch.x = x;
			        thisTouch.y = y;
			        raytraceThread.touches.add(thisTouch);
			        break;
			    }
			        
			    case MotionEvent.ACTION_MOVE: {
			    	int pointerIndex = ev.getPointerId(0);
			    	TouchTracker touch = null;
			    	for(int i=0; i < raytraceThread.touches.size(); i++) {
						TouchTracker thisTouch = raytraceThread.touches.get(i);
						if(pointerIndex == thisTouch.pointerID) {
							touch = thisTouch;
							break;
						}
					}
			    	if(touch == null)
			    		break;

			    	touch.x = ev.getX(0);
			    	touch.y = ev.getY(0);
			    	
			        break;
			    }
			        
			    case MotionEvent.ACTION_UP: {
			    	int pointerIndex = ev.getPointerId(0);
			    	int touchID = -1;
			    	for(int i=0; i < raytraceThread.touches.size(); i++) {
						TouchTracker thisTouch = raytraceThread.touches.get(i);
						if(pointerIndex == thisTouch.pointerID) {
							touchID = i;
							break;
						}
					}
			    	if(touchID == -1)
			    		break;

			    	raytraceThread.touches.remove(touchID);
			    	
			        break;
			    }
			        
			    case MotionEvent.ACTION_CANCEL: {
			        /*mActivePointerId = INVALID_POINTER_ID;
			        break;*/
			    }
			    
			    case MotionEvent.ACTION_POINTER_UP: {
			        // Extract the index of the pointer that left the touch sensor
			        /*final int pointerIndex = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) 
			                >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
			        final int pointerId = ev.getPointerId(pointerIndex);
			        if (pointerId == mActivePointerId) {
			            // This was our active pointer going up. Choose a new
			            // active pointer and adjust accordingly.
			            final int newPointerIndex = pointerIndex == 0 ? 1 : 0;
			            mLastTouchX = ev.getX(newPointerIndex);
			            mLastTouchY = ev.getY(newPointerIndex);
			            mActivePointerId = ev.getPointerId(newPointerIndex);
			        }
			        break;*/
			    }
			    }
			    
			    return true;
			}
		});

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
		raytraceThread.terminateThread=true;
	}

	class RaytraceTask extends AsyncTask<Void, Integer, Bitmap> {
		private boolean terminateThread = false;
		private boolean ClearStats = false;
		private long startTime;
		private long numRays;
		private int numFrames;
		ArrayList<TouchTracker> touches = new ArrayList<TouchTracker> ();

		public class TouchTracker {
			int pointerID;
			int sphereID;
			float x;
			float y;
		}

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
			while(!terminateThread) {
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

	/* load our native library */
	static {
		System.loadLibrary("plasma");
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
