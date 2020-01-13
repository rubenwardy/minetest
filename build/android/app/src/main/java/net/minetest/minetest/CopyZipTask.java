/*
Minetest
Copyright (C) 2014-2020 MoNTE48, Maksim Gamarnik <MoNTE48@mail.ua>
Copyright (C) 2014-2020 ubulem,  Bektur Mambetov <berkut87@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

package net.minetest.minetest;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.AsyncTask;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

class CopyZipTask extends AsyncTask<String, Void, String> {
	@SuppressLint("StaticFieldLeak")
	private final AppCompatActivity activity;
	private boolean isCancel = false;

	CopyZipTask(AppCompatActivity activity) {
		this.activity = activity;
	}

	protected String doInBackground(String... params) {
		while (!isCancel)
			copyAssets(params);
		return params[0];
	}

	@Override
	protected void onPostExecute(String result) {
		startUnzipService(result);
	}

	private void copyAsset(String zipName) throws IOException {
		String filename = zipName.substring(zipName.lastIndexOf("/") + 1);
		try (InputStream in = activity.getAssets().open(filename);
		     OutputStream out = new FileOutputStream(zipName)) {
			copyFile(in, out);
		}
	}

	private void copyAssets(String[] zips) {
		try {
			for (String zipName : zips)
				copyAsset(zipName);
			isCancel = true;
		} catch (IOException e) {
			Log.e("CopyZipTask", e.getLocalizedMessage());
			isCancel = true;
			cancel(true);
		}
	}

	private void copyFile(InputStream in, OutputStream out) throws IOException {
		byte[] buffer = new byte[1024];
		int read;
		while ((read = in.read(buffer)) != -1)
			out.write(buffer, 0, read);
	}

	private void startUnzipService(String file) {
		Intent intent = new Intent(activity, UnzipService.class);
		intent.putExtra(UnzipService.EXTRA_KEY_IN_FILE, file);
		activity.startService(intent);
	}
}
