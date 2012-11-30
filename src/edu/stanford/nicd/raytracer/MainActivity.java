
package edu.stanford.nicd.raytracer;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Spinner;
import android.widget.TextView;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

public class MainActivity extends Activity {
	
	private Bitmap mOriginal;
	private Bitmap mDecomposed;
	private Bitmap mRecomposed;
	private Bitmap mDifference;
	private float mappingConstant = 1;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.main);
		((RadioButton) findViewById(R.id.radioOriginal)).toggle();
		((CheckBox) findViewById(R.id.checkBoxMapping)).toggle();
		
		Spinner imageSelector = (Spinner) findViewById(R.id.imageSelector);
		imageSelector.setSelection(2);
		imageSelector.setOnItemSelectedListener(new OnItemSelectedListener() {
			public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
				recomputeImages();
			}
			public void onNothingSelected(AdapterView<?> arg0) {}
		});
		
		CheckBox checkBoxMapping = (CheckBox) findViewById(R.id.checkBoxMapping);
		checkBoxMapping.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				recomputeImages();	
			}
		});
		
		RadioGroup imageMode = (RadioGroup) findViewById(R.id.imageMode);
		imageMode.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
			public void onCheckedChanged(RadioGroup rGroup, int checkedId) {
				redisplayImages();
			}
		});

		SeekBar CompressionBar = (SeekBar) findViewById(R.id.CompressionBar);
		CompressionBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() { 
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {       
				mappingConstant = (float) progress/8;
			}
			public void onStartTrackingTouch(SeekBar seekBar) {}
			public void onStopTrackingTouch(SeekBar seekBar) {
				recomputeImages();
			}  
		});

		recomputeImages();
	}

	private void recomputeImages() {
		final BitmapFactory.Options options = new BitmapFactory.Options();
		options.inScaled = false;	// No pre-scaling
		Spinner imageSelector = (Spinner) findViewById(R.id.imageSelector);
		String selectedImage = String.valueOf(imageSelector.getSelectedItem());
		if(selectedImage.equals("Lena"))
			mOriginal = BitmapFactory.decodeResource(getResources(), R.drawable.lena, options);
		else if(selectedImage.equals("Mandrill"))
			mOriginal = BitmapFactory.decodeResource(getResources(), R.drawable.mandrill, options);
		else if(selectedImage.equals("Mona Lisa"))
			mOriginal = BitmapFactory.decodeResource(getResources(), R.drawable.monalisa, options);
		else if(selectedImage.equals("Noise"))
			mOriginal = BitmapFactory.decodeResource(getResources(), R.drawable.noise, options);
		else //if(selectedImage.equals("Tahoe"))
			mOriginal = BitmapFactory.decodeResource(getResources(), R.drawable.tahoe, options);
		mDecomposed = mOriginal.copy(Bitmap.Config.ARGB_8888, true);
		mRecomposed = mOriginal.copy(Bitmap.Config.ARGB_8888, true);
		mDifference = mOriginal.copy(Bitmap.Config.ARGB_8888, true);
		
		boolean doMap = ((CheckBox) findViewById(R.id.checkBoxMapping)).isChecked();
		
		((SeekBar) findViewById(R.id.CompressionBar)).setEnabled(doMap);
		((TextView) findViewById(R.id.CompressionFunction)).setEnabled(doMap);
		
		long CalculationTime = System.currentTimeMillis();
		int ptr = NonstandardWaveletDecompose(mOriginal);
		ExportWavelet(ptr, mDecomposed, 0);
		
		NonstandardWaveletRecompose(ptr, mRecomposed);
		CalculationTime = System.currentTimeMillis() - CalculationTime;
		((TextView) findViewById(R.id.CalculationTime)).setText("Calculation Time: " + Float.toString(CalculationTime) +" ms");
		ImageDifference(mRecomposed, mDifference);
		float PSNR = PSNR(mOriginal, mRecomposed);
		((TextView) findViewById(R.id.PSNR)).setText("PSNR: " + Float.toString(PSNR));
		((TextView) findViewById(R.id.CompressionFunction)).setText("Threshold " + Float.toString(mappingConstant));
		redisplayImages();
	}
	
	public void redisplayImages() {
		float entropy = -1;
		ImageView mImage = (ImageView) findViewById(R.id.imageView1);
		if(((RadioButton) findViewById(R.id.radioOriginal)).isChecked()) {
			mImage.setImageBitmap(mOriginal);
			entropy = CalculateEntropy(mOriginal);
		} else if(((RadioButton) findViewById(R.id.radioDecomposed)).isChecked()) {
			mImage.setImageBitmap(mDecomposed);
			entropy = CalculateEntropy(mDecomposed);
		} else if(((RadioButton) findViewById(R.id.radioRecomposed)).isChecked()) {
			mImage.setImageBitmap(mRecomposed);
			entropy = CalculateEntropy(mRecomposed);
		} else { //if(((RadioButton) findViewById(R.id.radioDifference)).isChecked()) {
			mImage.setImageBitmap(mDifference);
			entropy = CalculateEntropy(mDifference);
		}
		((TextView) findViewById(R.id.Entropy)).setText("Entropy: " + String.format("%.2f", entropy / 1000000) + "x10^6");
	}

	/* load our native library */
	static {
		System.loadLibrary("plasma");
	}

	private static native int NonstandardWaveletDecompose(Bitmap input);
	private static native void NonstandardWaveletRecompose(int ptr, Bitmap output);
	private static native void ExportWavelet(int ptr, Bitmap output, int compression);
	private static native void ImportWavelet(Bitmap output, int ptr);
	private static native void ImageDifference(Bitmap input, Bitmap output);
	private static native float PSNR(Bitmap reference, Bitmap reconstruction);
	private static native float CalculateEntropy(Bitmap input);
}
