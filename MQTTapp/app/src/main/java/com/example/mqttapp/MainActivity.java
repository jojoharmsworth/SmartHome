package com.example.mqttapp;

import androidx.appcompat.app.AppCompatActivity;
import androidx.navigation.NavController;
import androidx.navigation.fragment.NavHostFragment;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationClientOption;
import com.amap.api.location.AMapLocationListener;
import com.google.android.material.bottomnavigation.BottomNavigationView;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.io.IOException;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class MainActivity extends AppCompatActivity implements AMapLocationListener {

    private static final String KEY_PUBLISH_TOPIC = "kylinBoard";
    private static final String TAG = "amap";
    private static final String KEY_AMAP_LOCATION = "aMapLocation";
    private static final String KEY_NAME_LOCATION_CITY = "nameLocationCity";
    private static final String ACTION_MAINACTIVITY_TO_HOMEFRAGMENT = "STM32_to_APP";
    private static final String ACTION_HOMEFRAGMENT_TO_MAINACTIVITY = "APP_to_STM32";
    private static final String KEY_SUBSCRIBE = "data_mqtt";
    private static final String KEY_PUBLISH = "data_app";
    private NavController navController;
    private ScheduledExecutorService scheduler;
    private MqttClient client;
    private Handler handler;
    private String host = "tcp://39.98.92.2:1883";
    private String mqtt_id = "android";
    private String userName = "jojo";
    private String passWord = "harmsworth";
    private String mqtt_sub_topic = "pcTopic";
    private int ledStatus = 0;
    private String dataLatitudeLongitude;
    private String nameLocationCity;
    private AMapLocationClient locationClientContinue;
    private AMapLocationClientOption locationClientContinueOption;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        BottomNavigationInit();

        Mqtt_init();
        startReconnect();
        registerBoardCast();
        //初始化高德定位
        initAMapLocation();

        handler = new Handler(Looper.myLooper()) {
            @SuppressLint("SetTextI18n")
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case 1: //开机校验更新回传
                        break;
                    case 2:  // 反馈回传

                        break;
                    case 3:  //MQTT 收到消息回传   UTF8Buffer msg=new UTF8Buffer(object.toString());

                        System.out.println(msg.obj.toString());   // 显示MQTT数据
                        Intent intent = new Intent();
                        intent.setAction(ACTION_MAINACTIVITY_TO_HOMEFRAGMENT);
                        intent.putExtra(KEY_SUBSCRIBE, msg.obj.toString());
                        sendBroadcast(intent);
                        break;
                    case 4:     //MQTT 消息发布
                        String str = String.format("{\r\n\"LED\": %d\r\n}", ledStatus);
                        PublishMessagePlus(KEY_PUBLISH_TOPIC, str);
                        System.out.println("led changed..." + str);
                        break;
                    case 30:  //连接失败
                        Toast.makeText(MainActivity.this, "连接失败", Toast.LENGTH_SHORT).show();
                        break;
                    case 31:   //连接成功
                        Toast.makeText(MainActivity.this, "连接成功", Toast.LENGTH_SHORT).show();
                        try {
                            client.subscribe(mqtt_sub_topic, 1);
                        } catch (MqttException e) {
                            e.printStackTrace();
                        }
                        break;
                    default:
                        break;
                }
            }
        };
    }

    @Override
    protected void onStop() {
        super.onStop();
        //停止定位
        locationClientContinue.stopLocation();//停止定位后，本地定位服务并不会被销毁
    }

    private void registerBoardCast() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_HOMEFRAGMENT_TO_MAINACTIVITY);
        registerReceiver(broadcastReceiver, intentFilter);
    }

    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                Message message = new Message();
                message.what = 4;
                ledStatus = intent.getBooleanExtra(KEY_PUBLISH, false) ? 1 : 0;
                handler.sendMessage(message);
            }
        }
    };

    private void BottomNavigationInit() {
        BottomNavigationView bottomNavigationView = findViewById(R.id.bottom_navigation_view);
        NavHostFragment navHostFragment =
                (NavHostFragment) getSupportFragmentManager().findFragmentById(R.id.fragmentContainerView);
        navController = navHostFragment.getNavController();

        AppBarConfiguration configuration = new AppBarConfiguration.Builder(bottomNavigationView.getMenu()).build();
        NavigationUI.setupWithNavController(bottomNavigationView, navController);
    }

    // MQTT初始化
    private void Mqtt_init() {
        try {
            //host为主机名，test为client id即连接MQTT的客户端ID，一般以客户端唯一标识符表示，MemoryPersistence设置clientid的保存形式，默认为以内存保存
            client = new MqttClient(host, mqtt_id,
                    new MemoryPersistence());
            //MQTT的连接设置
            MqttConnectOptions options = new MqttConnectOptions();
            //设置是否清空session,这里如果设置为false表示服务器会保留客户端的连接记录，这里设置为true表示每次连接到服务器都以新的身份连接
            options.setCleanSession(false);
            //设置连接的用户名
            options.setUserName(userName);
            //设置连接的密码
            options.setPassword(passWord.toCharArray());
            // 设置超时时间 单位为秒
            options.setConnectionTimeout(10);
            // 设置会话心跳时间 单位为秒 服务器会每隔1.5*20秒的时间向客户端发送个消息判断客户端是否在线，但这个方法并没有重连的机制
            options.setKeepAliveInterval(20);
            //设置回调
            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    //连接丢失后，一般在这里面进行重连
                    System.out.println("connectionLost----------");
                    //startReconnect();
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    //publish后会执行到这里
                    System.out.println("deliveryComplete---------"
                            + token.isComplete());
                }

                @Override
                public void messageArrived(String topicName, MqttMessage message)
                        throws Exception {
                    //subscribe后得到的消息会执行到这里面
                    System.out.println("messageArrived----------");
                    Message msg = new Message();
                    msg.what = 3;   //收到消息标志位
//                    msg.obj = topicName + "---" +message.toString();
                    msg.obj = message.toString();
                    handler.sendMessage(msg);    // handler 回传
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // MQTT连接函数
    private void Mqtt_connect() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    if (!(client.isConnected()))  //如果还未连接
                    {
                        MqttConnectOptions options = null;
                        client.connect(options);
                        Message msg = new Message();
                        msg.what = 31;
                        handler.sendMessage(msg);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    Message msg = new Message();
                    msg.what = 30;
                    handler.sendMessage(msg);
                }
            }
        }).start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        //注销广播接收
        unregisterReceiver(broadcastReceiver);
        //销毁定位客户端
        locationClientContinue.onDestroy();//销毁定位客户端，同时销毁本地定位服务。
    }

    // MQTT重新连接函数
    private void startReconnect() {
        scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleAtFixedRate(new Runnable() {
            @Override
            public void run() {
                if (!client.isConnected()) {
                    Mqtt_connect();
                }
            }
        }, 0 * 1000, 10 * 1000, TimeUnit.MILLISECONDS);
    }

    // 订阅函数    (下发任务/命令)
    private void PublishMessagePlus(String topic, String message2) {
        if (client == null || !client.isConnected()) {
            return;
        }
        MqttMessage message = new MqttMessage();
        message.setPayload(message2.getBytes());
        try {
            client.publish(topic, message);
        } catch (MqttException e) {

            e.printStackTrace();
        }
    }

    /**
     * 初始化高德定位
     * <p/>
     *
     * @param
     * @return void
     * @author jojo
     * @date 2023/3/14
     */
    private void initAMapLocation() {

        AMapLocationClient.updatePrivacyAgree(this, true);
        AMapLocationClient.updatePrivacyShow(this, true, true);
        try {
            locationClientContinue = new AMapLocationClient(this);
            locationClientContinueOption = new AMapLocationClientOption();

            //设置定位模式为AMapLocationMode.Battery_Saving，低功耗模式。
            locationClientContinueOption.setLocationMode(AMapLocationClientOption.AMapLocationMode.Battery_Saving);
            //设置定位间隔,单位毫秒,默认为2000ms，最低1000ms。
            locationClientContinueOption.setInterval(5000);
            //设置是否返回地址信息（默认返回地址信息）
            locationClientContinueOption.setNeedAddress(true);
            //单位是毫秒，默认30000毫秒，建议超时时间不要低于8000毫秒。
            locationClientContinueOption.setHttpTimeOut(20000);
            //关闭缓存机制
            locationClientContinueOption.setLocationCacheEnable(false);
            //给定位客户端对象设置定位参数
            locationClientContinue.setLocationOption(locationClientContinueOption);
            //启动定位
            locationClientContinue.startLocation();

            locationClientContinue.setLocationListener(this);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * 高德定位回调
     * <p/>
     *
     * @param aMapLocation
     * @return void
     * @author jojo
     * @date 2023/3/14
     */
    @SuppressLint("DefaultLocale")
    @Override
    public void onLocationChanged(AMapLocation aMapLocation) {
        if (aMapLocation != null) {
            if (aMapLocation.getErrorCode() == 0) {
                //可在其中解析aMapLocation获取相应内容。
                dataLatitudeLongitude = String.format("%.2f,%.2f", aMapLocation.getLongitude(), aMapLocation.getLatitude());
                nameLocationCity = aMapLocation.getCity();
                Log.d(TAG, "onLocationChanged: city: " + nameLocationCity);
                System.out.println(nameLocationCity);
                Log.d(TAG, "onLocationChanged: latitude" + String.valueOf(aMapLocation.getLatitude()));
                Log.d(TAG, "onLocationChanged: Longitude" + String.valueOf(aMapLocation.getLongitude()));
                //todo: 得到定位后，刷新天气
                Intent intent = new Intent();
                intent.setAction(ACTION_MAINACTIVITY_TO_HOMEFRAGMENT);
                intent.putExtra(KEY_AMAP_LOCATION, dataLatitudeLongitude);
                intent.putExtra(KEY_NAME_LOCATION_CITY, nameLocationCity);
                sendBroadcast(intent);

            } else {
                //定位失败时，可通过ErrCode（错误码）信息来确定失败的原因，errInfo是错误信息，详见错误码表。
                Log.e("AmapError", "location Error, ErrCode:"
                        + aMapLocation.getErrorCode() + ", errInfo:"
                        + aMapLocation.getErrorInfo());
            }
        }
    }
}