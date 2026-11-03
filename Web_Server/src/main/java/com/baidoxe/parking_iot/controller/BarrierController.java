package com.baidoxe.parking_iot.controller;

import com.baidoxe.parking_iot.mqtt.MqttSubscriber;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.Map;

@RestController
@RequestMapping("/api/barrier")
@CrossOrigin(origins = "*")
public class BarrierController {

    @Autowired
    private MqttSubscriber mqttSubscriber;

    // API mới tinh nhận lệnh khẩn cấp O1, C1, O2, C2
    @PostMapping("/emergency/{command}")
    public ResponseEntity<Map<String, Object>> emergencyControl(@PathVariable String command) {
        Map<String, Object> response = new HashMap<>();
        try {
            // Bắn đúng cái chữ (command) nhận được lên topic mới
            mqttSubscriber.publishMessage("/ptit/servo/emergency", command);
            
            response.put("success", true);
            response.put("message", "Đã gửi thành công lệnh khẩn cấp: " + command);
            return ResponseEntity.ok(response);
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "Lỗi gửi lệnh điều khiển!");
            return ResponseEntity.status(500).body(response);
        }
    }
}