package com.example.mqttapp;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationClientOption;
import com.amap.api.location.AMapLocationListener;
import com.google.gson.Gson;
import com.qweather.sdk.bean.base.Code;
import com.qweather.sdk.bean.base.Lang;
import com.qweather.sdk.bean.base.Unit;
import com.qweather.sdk.bean.weather.WeatherNowBean;
import com.qweather.sdk.view.HeConfig;
import com.qweather.sdk.view.QWeather;

import org.json.JSONException;
import org.json.JSONObject;

import java.text.Format;

/**
 * A simple {@link Fragment} subclass.
 * Use the {@link HomeFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class HomeFragment extends Fragment {

    // TODO: Rename parameter arguments, choose names that match
    // the fragment initialization parameters, e.g. ARG_ITEM_NUMBER
    private static final String ARG_PARAM1 = "param1";
    private static final String ARG_PARAM2 = "param2";

    private static final String ACTION_MAINACTIVITY_TO_HOMEFRAGMENT = "STM32_to_APP";
    private static final String ACTION_HOMEFRAGMENT_TO_MAINACTIVITY = "APP_to_STM32";
    private static final String KEY_AMAP_LOCATION = "aMapLocation";
    private static final String KEY_NAME_LOCATION_CITY = "nameLocationCity";
    private static final String KEY_SUBSCRIBE = "data_mqtt";
    private static final String KEY_PUBLISH = "data_app";
    private static final String KEY_RED_LED = "led_status";
    private static final String KEY_PUBLIC_ID = "HE2303131124251833";
    private static final String KEY_KEY = "0ec22918191840caafd73c7c53bca2f3";
    private static final String TAG = "amap";

    private final String FILE_NAME = "App_data";             //???????????????

    // TODO: Rename and change types of parameters
    private String mParam1;
    private String mParam2;

    private static final String KEY_TEMPERATURE = "temp";
    private static final String KEY_HUMIDITY = "humi";
    private static final String KEY_ILLUMINATION = "illumination";

    private static int temper;
    private static int humi;
    private static int illum;
    private TextView txvLocationCity;
    private TextView txvTemperature;
    private TextView txvIllumination;
    private TextView txvHumidity;
    private TextView txvWeatherTemperature;
    private TextView txvWeatherStatus;
    private TextView txvWindSpeed;
    private TextView txvObsTime;
    private Switch swLed;

    public Handler handler;
    private JSONObject jsonObject;
    private boolean fragmentViewDestroyFlag = false;
    private int ledR;

    public AMapLocationClient aMapLocationClient = null;
    private AMapLocationClient locationClientContinue;
    private AMapLocationClientOption locationClientContinueOption;
    private String nameLocationCity;
    private String dataLatitudeLongitude;

    //TODO: ????????????????????????

    public HomeFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @param param1 Parameter 1.
     * @param param2 Parameter 2.
     * @return A new instance of fragment HomeFragment.
     */
    // TODO: Rename and change types and number of parameters
    public static HomeFragment newInstance(String param1, String param2) {
        HomeFragment fragment = new HomeFragment();
        Bundle args = new Bundle();
        args.putString(ARG_PARAM1, param1);
        args.putString(ARG_PARAM2, param2);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            mParam1 = getArguments().getString(ARG_PARAM1);
            mParam2 = getArguments().getString(ARG_PARAM2);
        }
        //????????????
        mRegisterBroadCast();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_home, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        fragmentViewDestroyFlag = false;

        handler = new Handler(Looper.myLooper()) {
            @SuppressLint("SetTextI18n")
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case 1:  //??????UI
                        refreshUi();
                        break;
                    case 3:  //MQTT ??????????????????   UTF8Buffer msg=new UTF8Buffer(object.toString());
                        try {
                            jsonObject = new JSONObject(msg.obj.toString());
                            temper = jsonObject.getInt(KEY_TEMPERATURE);
                            humi = jsonObject.getInt(KEY_HUMIDITY);
                            illum = jsonObject.getInt(KEY_ILLUMINATION);
                            ledR = jsonObject.getInt(KEY_RED_LED);
                        } catch (JSONException e) {
                            if (e.getMessage().matches("No value for temper")) {
                                temper = 99;
                            }
                            if (e.getMessage().matches("No value for humi")) {
                                humi = 99;
                            }
                            if (e.getMessage().matches("No value for illum")) {
                                illum = 99;
                            }
                        }
                        refreshUi();
                        break;
                    default:
                        break;
                }
            }
        };

        load();                 //????????????
        initView(view);
        //?????????????????????
        HeConfig.init(KEY_PUBLIC_ID, KEY_KEY);
        HeConfig.switchToDevService();

    }

    private final BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null && !fragmentViewDestroyFlag) {
                //??? MainActivity ????????????????????????
                if (intent.getStringExtra(KEY_NAME_LOCATION_CITY) != null && intent.getStringExtra(KEY_AMAP_LOCATION) != null) {
                    dataLatitudeLongitude = intent.getStringExtra(KEY_AMAP_LOCATION);
                    nameLocationCity = intent.getStringExtra(KEY_NAME_LOCATION_CITY);
                    setTempAndHumidity();
                }
                //??? MainActivity ?????? Mqtt ??????????????????
                if (intent.getStringExtra(KEY_SUBSCRIBE) != null) {
                    Message message = new Message();
                    message.what = 3;
                    message.obj = intent.getStringExtra(KEY_SUBSCRIBE);
                    handler.sendMessage(message);
                }
            }
        }
    };

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        fragmentViewDestroyFlag = true;
    }

    @Override
    public void onStop() {
        super.onStop();
        save();

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Toast.makeText(getContext(), "onDestroy", Toast.LENGTH_SHORT).show();
        getContext().unregisterReceiver(broadcastReceiver);
    }

    private void mRegisterBroadCast() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_MAINACTIVITY_TO_HOMEFRAGMENT);
        getContext().registerReceiver(broadcastReceiver, intentFilter);
    }

    private void save() {
        //??????????????????
        SharedPreferences shp = getActivity().getApplication().getSharedPreferences(FILE_NAME, Context.MODE_PRIVATE);
        //????????? ????????????
        SharedPreferences.Editor editor = shp.edit();
        editor.putInt(KEY_TEMPERATURE, temper);
        editor.putInt(KEY_HUMIDITY, humi);
        editor.putInt(KEY_ILLUMINATION, illum);
//        editor.putInt(KEY_RED_LED, ledR);
        //???????????????
        editor.apply();
    }

    private void load() {
        //??????????????????????????????
        SharedPreferences shp = getActivity().getApplication().getSharedPreferences(FILE_NAME, Context.MODE_PRIVATE);
        //??????????????? KEY ???????????????handle
        //???????????????????????? ??? 0
        temper = shp.getInt(KEY_TEMPERATURE, 99);
        humi = shp.getInt(KEY_HUMIDITY, 99);
        illum = shp.getInt(KEY_ILLUMINATION, 99);
//        ledR = shp.getInt(KEY_RED_LED, 0);
    }

    /**
     * ?????????UI
     * <p/>
     *
     * @param view view
     * @return void
     * @author jojo
     * @date 2023/3/14
     */
    @SuppressLint("SetTextI18n")
    private void initView(@NonNull View view) {
        txvTemperature = view.findViewById(R.id.data_temperature);
        txvHumidity = view.findViewById(R.id.data_humidity);
        txvIllumination = view.findViewById(R.id.data_illumination);
        txvWeatherTemperature = view.findViewById(R.id.weather_temperature);
        txvWeatherStatus = view.findViewById(R.id.weather_status);
        txvLocationCity = view.findViewById(R.id.location_city);
        txvWindSpeed = view.findViewById(R.id.wind_speed);
        txvObsTime = view.findViewById(R.id.obs_time);
        swLed = view.findViewById(R.id.sw_led);

        swLed.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Intent intent = new Intent();
                intent.setAction(ACTION_HOMEFRAGMENT_TO_MAINACTIVITY);
                intent.putExtra(KEY_PUBLISH, isChecked);
                getActivity().sendBroadcast(intent);
            }
        });

        Message message = new Message();
        message.what = 1;
        handler.sendMessage(message);
    }

    /**
     * ??????ui
     * <p/>
     *
     * @param
     * @return void
     * @author
     * @date 2023/3/14
     */
    private void refreshUi() {
        txvTemperature.setText(temper + "??");
        txvHumidity.setText(humi + "%");
        txvIllumination.setText((illum) + "lx");
        txvLocationCity.setText(nameLocationCity);
        swLed.setChecked(ledR == 1);
    }

    /**
     * <p/> ???API??????????????????
     *
     * @param
     * @return void
     * @author jojo
     * @date 2023-3-13
     */
    public void setTempAndHumidity() {
        //location:????????????????????????????????????ID??????????????????????????????????????????????????????????????????????????????????????????"CN101180101"
        //location?????????????????????????????????????????????
        QWeather.getWeatherNow(getActivity(), dataLatitudeLongitude, Lang.ZH_HANS, Unit.METRIC,
                new QWeather.OnResultWeatherNowListener() {
                    public static final String TAG = "he_feng_now";

                    @Override
                    public void onError(Throwable e) {
                        Log.i(TAG, "onError: ", e);
                        System.out.println("??????????????????");
                        System.out.println("Weather Now Error:" + new Gson());
                    }

                    @Override
                    public void onSuccess(WeatherNowBean weatherBean) {
                        //Log.i(TAG, "getWeather onSuccess: " + new Gson().toJson(weatherBean));
                        System.out.println("????????????????????? " + new Gson().toJson(weatherBean));
                        //??????????????????status??????????????????status???????????????????????????status?????????????????????status?????????Code???????????????

                        if (Code.OK == weatherBean.getCode()) {
                            WeatherNowBean.NowBaseBean now = weatherBean.getNow();
                            System.out.println(now);
                            String tianqi = now.getText();//??????
                            String wendu = now.getTemp() + "???";//??????
                            String fengli = now.getWindScale();//??????
                            String obsTime = now.getObsTime();//????????????
                            String fengxiang = now.getWindDir();//??????
                            String shidu = now.getHumidity() + "%";//??????


                            getActivity().runOnUiThread(new Runnable() {
                                public void run() {
                                    txvWeatherStatus.setText(tianqi);//??????????????????
                                    txvWeatherTemperature.setText(wendu);//??????????????????
                                    txvWindSpeed.setText("?????????" + fengli + "??????/??????");//??????????????????
                                    txvObsTime.setText("?????????????????????" + obsTime);
                                }
                            });
                    /*?????????????????????????????????????????????getActivity()...void run(){}??????
                    ?????????????????????Fragment?????????????????????????????????????????????????????????
                    ???Activity???????????????????????????????????????????????????runOnUi...
                    ?????????https://blog.csdn.net/i_nclude/article/details/105563688*/

                        } else {
                            //???????????????????????????????????????
                            Code code = weatherBean.getCode();
                            System.out.println("????????????: " + code);
                            //Log.i(TAG, "failed code: " + code);
                        }
                    }
                });
    }
}