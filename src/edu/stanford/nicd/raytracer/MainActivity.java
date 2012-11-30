
package edu.stanford.nicd.raytracer;

import android.app.Activity;
import android.os.Bundle;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

public class MainActivity extends Activity {
	
	private Bitmap mImage;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.main);
		((RadioButton) findViewById(R.id.radioOriginal)).toggle();
		((CheckBox) findViewById(R.id.checkBoxMapping)).toggle();

		recomputeImages();
	}

	private void recomputeImages() {
		final BitmapFactory.Options options = new BitmapFactory.Options();
		options.inScaled = false;	// No pre-scaling
		mImage = BitmapFactory.decodeResource(getResources(), R.drawable.tahoe, options);
		NonstandardWaveletRecompose(mImage);
		ImageView mImageView = (ImageView) findViewById(R.id.imageView1);
		mImageView.setImageBitmap(mImage);
	}

	/* load our native library */
	static {
		System.loadLibrary("plasma");
	}

	private static native void NonstandardWaveletRecompose(Bitmap output);
}
