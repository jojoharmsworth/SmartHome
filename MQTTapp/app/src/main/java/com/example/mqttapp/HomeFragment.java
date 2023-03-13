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
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import org.eclipse.paho.client.mqttv3.MqttClient;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.concurrent.ScheduledExecutorService;

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
    private static final String KEY_SUBSCRIBE = "data_mqtt";
    private static final String KEY_PUBLISH = "data_app";
    private static final String KEY_RED_LED = "led_status";

    private final String FILE_NAME = "App_data";             //数据文件名

    // TODO: Rename and change types of parameters
    private String mParam1;
    private String mParam2;

    private static final String KEY_TEMPERATURE = "temp";
    private static final String KEY_HUMIDITY = "humi";
    private static final String KEY_ILLUMINATION = "illumination";

    private static int temper;
    private static int humi;
    private static int illum;
    private TextView txvTemperature;
    private TextView txvIllumination;
    private TextView txvHumidity;
    private Switch swLed;

    public Handler handler;
    private JSONObject jsonObject;
    private boolean fragmentViewDestroyFlag = false;
    private int ledR;

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
        //注册广播
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

        load();                 //加载数据
        initView(view);
        handler = new Handler(Looper.myLooper()) {
            @SuppressLint("SetTextI18n")
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case 3:  //MQTT 收到消息回传   UTF8Buffer msg=new UTF8Buffer(object.toString());
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
    }

    private final BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null && !fragmentViewDestroyFlag) {
                Message message = new Message();
                message.what = 3;
                message.obj = intent.getStringExtra(KEY_SUBSCRIBE);
                handler.sendMessage(message);
            }
        }
    };

    @SuppressLint("SetTextI18n")
    private void initView(@NonNull View view) {
        txvTemperature = view.findViewById(R.id.data_temperature);
        txvHumidity = view.findViewById(R.id.data_humidity);
        txvIllumination = view.findViewById(R.id.data_illumination);
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

        refreshUi();
    }

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

    private void refreshUi() {
        txvTemperature.setText(temper + "°");
        txvHumidity.setText(humi + "%");
        txvIllumination.setText((illum) + "lx");
        swLed.setChecked(ledR == 1);
    }

    private void mRegisterBroadCast() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_MAINACTIVITY_TO_HOMEFRAGMENT);
        getContext().registerReceiver(broadcastReceiver, intentFilter);
    }

    private void save() {
        //获取文件数据
        SharedPreferences shp = getActivity().getApplication().getSharedPreferences(FILE_NAME, Context.MODE_PRIVATE);
        //对数据 进行编辑
        SharedPreferences.Editor editor = shp.edit();
        editor.putInt(KEY_TEMPERATURE, temper);
        editor.putInt(KEY_HUMIDITY, humi);
        editor.putInt(KEY_ILLUMINATION, illum);
//        editor.putInt(KEY_RED_LED, ledR);
        //编辑后保存
        editor.apply();
    }

    private void load() {
        //从对应文件里获取数据
        SharedPreferences shp = getActivity().getApplication().getSharedPreferences(FILE_NAME, Context.MODE_PRIVATE);
        //把关键字为 KEY 的数据传给handle
        //如果没有数据，则 给 0
        temper = shp.getInt(KEY_TEMPERATURE, 99);
        humi = shp.getInt(KEY_HUMIDITY, 99);
        illum = shp.getInt(KEY_ILLUMINATION, 99);
//        ledR = shp.getInt(KEY_RED_LED, 0);
    }
}