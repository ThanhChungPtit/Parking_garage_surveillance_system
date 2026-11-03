package com.baidoxe.parking_iot.mqtt;

import com.baidoxe.parking_iot.entity.ParkingSpot;
import com.baidoxe.parking_iot.repository.ParkingSpotRepository;
import jakarta.annotation.PostConstruct;
import org.eclipse.paho.client.mqttv3.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Map;

@Service
public class MqttSubscriber implements MqttCallback {

    private static final String BROKER_URL = "tcp://broker.emqx.io:1883";
    private static final String CLIENT_ID = MqttClient.generateClientId();
    
    private MqttClient client;

    @Autowired
    private ParkingSpotRepository parkingSpotRepository;

    // Gọi Controller của thẻ RFID vào để xử lý tính tiền
    @Autowired
    private com.baidoxe.parking_iot.controller.RfidController rfidController;

    @PostConstruct
    public void startListening() {
        try {
            client = new MqttClient(BROKER_URL, CLIENT_ID);
            client.setCallback(this);
            
            MqttConnectOptions options = new MqttConnectOptions();
            options.setCleanSession(true);
            
            client.connect(options);
            
            // ĐĂNG KÝ NGHE 2 KÊNH
            client.subscribe("/state/park/#");         // Kênh 1: Cảm biến ô đỗ
            client.subscribe("/ptit/parking/check");   // Kênh 2: Đầu đọc thẻ RFID
            
            System.out.println("Đã kết nối MQTT! Đang nghe cảm biến và thẻ RFID...");
        } catch (MqttException e) {
            System.err.println("Lỗi kết nối MQTT: " + e.getMessage());
        }
    }

    @Override
    public void connectionLost(Throwable cause) {
        System.out.println("Mất kết nối MQTT! Lý do: " + cause.getMessage());
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) throws Exception {
        String payload = new String(message.getPayload()).trim();

        // ==========================================
        // TRƯỜNG HỢP 1: CẢM BIẾN VỊ TRÍ ĐỖ BÁO VỀ
        // ==========================================
        if (topic.startsWith("/state/park/")) {
            String[] parts = topic.split("/");
            if (parts.length >= 4) {
                String sensorId = parts[3]; 
                ParkingSpot spot = parkingSpotRepository.findBySensorId(sensorId);
                
                if (spot != null) {
                    boolean isOccupied = payload.equals("0"); // "0" là CÓ XE
                    spot.setIsOccupied(isOccupied);
                    parkingSpotRepository.save(spot);
                }
            }
        } 
        
        // ==========================================
        // TRƯỜNG HỢP 2: THẺ RFID VỪA ĐƯỢC QUẸT
        // ==========================================
        else if (topic.equals("/ptit/parking/check")) {
            System.out.println("💳 Vừa quẹt thẻ UID: " + payload);
            
            // Gọi hàm xử lý (Hàm này tự động biết là Vào hay Ra nhờ Database)
            Map<String, Object> result = rfidController.scanCard(payload);
            System.out.println("   -> Kết quả: " + result.get("message"));
            
            // Nếu Java duyệt cho qua (thành công)
            if (result.containsKey("success") && (Boolean) result.get("success")) {
                
                // Lấy ra hành động là ENTRY (Vào) hay EXIT (Ra)
                String action = (String) result.get("action");
                
                if ("ENTRY".equals(action)) {
                    // XE VÀO: Bắn số "1" lên kênh điều khiển
                    client.publish("/ptit/servo/control", new MqttMessage("1".getBytes()));
                    System.out.println("🚧 Đã bắn lệnh 1 (MỞ BARIE VÀO)!");
                } 
                else if ("EXIT".equals(action)) {
                    // XE RA: Bắn số "2" lên kênh điều khiển
                    client.publish("/ptit/servo/control", new MqttMessage("2".getBytes()));
                    System.out.println("🚧 Đã bắn lệnh 2 (MỞ BARIE RA)!");
                }
            }
        }
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
    }

    // Hàm này cho phép Controller gọi đến để bắn tin nhắn lên MQTT
    public void publishMessage(String topic, String payload) {
        try {
            if (client != null && client.isConnected()) {
                client.publish(topic, new MqttMessage(payload.getBytes()));
                System.out.println("📤 Đã gửi lệnh THỦ CÔNG -> Topic: " + topic + " | Lệnh: " + payload);
            } else {
                System.out.println("❌ Lỗi: MQTT chưa kết nối!");
            }
        } catch (MqttException e) {
            System.err.println("❌ Lỗi gửi MQTT: " + e.getMessage());
        }
    }
}