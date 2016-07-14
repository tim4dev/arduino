package com.tim4dev.weatherstation;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.Html;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Iterator;

public class MainActivity extends AppCompatActivity {

    private TextView tvData;
    private Button   btRefresh;
    private EditText etUrl;
    private TextView tvLink;

    // изменить URL по умолчанию !!!
    private static final String defUrl = "http://host/dir/last-data-to-json.php?k=!!!";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvData = (TextView) findViewById(R.id.tvData);
        btRefresh = (Button) findViewById(R.id.btRefresh);

        etUrl = (EditText) findViewById(R.id.etUrl);

        tvLink = (TextView) findViewById(R.id.tvLink);
        tvLink.setText(Html.fromHtml("© <a href=\"http://tim4dev.com/tag/meteostantsiya/\">Проект \"Метеостанция от А до Я\" на tim4dev.com</a>"));

        // загружаем настройки
        loadSettings();
    }

    // Вызывается AsyncTask
    // Перед запросом идёт проверка доступности подключения к сети
    public void myRefreshClickHandler(View view) {
        ConnectivityManager connMgr = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();
        if (networkInfo != null && networkInfo.isConnected()) {
            // запускаем REST запрос
            new getRestData().execute( etUrl.getText().toString() );
        } else {
            tvData.setText("Нет подключения к сети.");
        }
    }

    /**
     * Работа с сетью, получение данных с веб-сайта
     */
    /*
    Используем AsyncTask, чтобы создать фоновую задачу отдельно от главного потока пользовательского интерфейса.
    Эта задача берет URL и использует его для создания HttpURLConnection.
    После того, как соединение  установлено, AsyncTask загружает содержимое веб-страницы как InputStream.
    Далее InputStream преобразуется в строку, которая отображается в пользовательском интерфейсе методом onPostExecute.
     */
    private class getRestData extends AsyncTask<String, Void, String> {

        @Override
        protected void onPreExecute() {
            tvData.setText("Запрос данных. Ожидайте...");
        }

        @Override
        protected String doInBackground(String... urls) {
            // параметры от вызова execute() : params[0] это  url
            try {
                return getRestData(urls[0]);
            } catch (IOException e) {
                return "Ошибка. Данные от веб-сервера не получены.";
            }
        }
        // onPostExecute показывает результат от AsyncTask.
        @Override
        protected void onPostExecute(String result) {
            tvData.setText(result);
        }
    }

    // Получаем всю содержимое веб-странички как InputStream и отдаем как строку
    private String getRestData(String strUrl) throws IOException {
        InputStream is = null;
        String contentAsString = "";
        // Получаем только первые len символов содержимого
        int len = 2048;

        try {
            URL url = new URL(strUrl);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setReadTimeout(10000 /* milliseconds */);
            conn.setConnectTimeout(15000 /* milliseconds */);
            conn.setRequestMethod("GET");
            conn.setDoInput(true);
            // Starts the query
            conn.connect();
            int response = conn.getResponseCode();
            is = conn.getInputStream();

            // Convert the InputStream into a string
            contentAsString = myInputStreamToString(is, len);

            try {
                String resStr = "";
                JSONObject jsonMain = new JSONObject(contentAsString);
                Iterator iterator = jsonMain.keys();
                while ( iterator.hasNext() ) {
                    String key = (String) iterator.next();
                    JSONObject json = jsonMain.getJSONObject(key);
                    resStr = resStr + "№ \t" + json.getString("idSensor") + ", \t" +
                            "Дата \t" + json.getString("dateCreate") + "\n";
                    if ( json.has("temperature") ) {
                        resStr = resStr + "Температура \t" + json.getString("temperature") + " °C\n";
                    }
                    if ( json.has("humidity") ) {
                        resStr = resStr + "Влажность \t" + json.getString("humidity") + " %\n";
                    }
                    if ( json.has("pressure") ) {
                        resStr = resStr + "Давление \t" + json.getString("pressure") + "\n";
                    }
                    if ( json.has("voltage") ) {
                        resStr = resStr + "Напряжение \t" + json.getString("voltage") + " В\n";
                    }
                    resStr = resStr + "---------------\n";
                }
                return resStr;
            } catch (JSONException e) {
                //e.printStackTrace();
                return "Ошибка. При разборе JSON.\n" + contentAsString;
            }
            // Гарантирует, что InputStream закрывается после того, как приложение закончит работу с ним
        } finally {
            if (is != null) {
                is.close();
            }
        }

    }


    // Reads an InputStream and converts it to a String.
    public String myInputStreamToString(InputStream stream, int len) throws IOException, UnsupportedEncodingException {
        Reader reader = null;
        reader = new InputStreamReader(stream, "UTF-8");
        char[] buffer = new char[len];
        reader.read(buffer);
        return new String(buffer);
    }


    /*
     * Сохранение настроек
     */
    private void saveSettings() {
        SharedPreferences sPref = getSharedPreferences("WeatherStation", MODE_PRIVATE);
        SharedPreferences.Editor ed = sPref.edit();
        ed.putString("REST_URL", etUrl.getText().toString());
        ed.commit();
    }

    /*
     * Чтение настроек
     */
    private void loadSettings() {
        SharedPreferences sPref = getSharedPreferences("WeatherStation", MODE_PRIVATE);
        String savedText = sPref.getString("REST_URL", "");

        if (savedText.length() <= 0)    {
            savedText = defUrl;
        }
        etUrl.setText(savedText);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        saveSettings();
    }

}
