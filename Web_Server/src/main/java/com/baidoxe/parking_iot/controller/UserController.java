package com.baidoxe.parking_iot.controller;

import com.baidoxe.parking_iot.entity.User;
import com.baidoxe.parking_iot.repository.UserRepository;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.Map;

@RestController
@RequestMapping("/api/users")
@CrossOrigin(origins = "*") // Mở cửa cho Frontend gọi sang thoải mái
public class UserController {

    private UserRepository userRepository = new UserRepository();

    // ================= 1. API LẤY DANH SÁCH NHÂN VIÊN =================
    @GetMapping
    public ResponseEntity<?> getAllUsers() {
        return ResponseEntity.ok(userRepository.findAll());
    }

    // ================= 2. API THÊM NHÂN VIÊN MỚI =================
    @PostMapping
    public ResponseEntity<Map<String, Object>> addUser(@RequestBody User newUser) {
        Map<String, Object> response = new HashMap<>();
        try {
            User existUser = userRepository.findByUsername(newUser.getUsername());
            if (existUser != null) {
                response.put("success", false);
                response.put("message", "Tên đăng nhập da ton tai!");
                return ResponseEntity.status(400).body(response);
            }
            
            userRepository.save(newUser);
            
            response.put("success", true);
            response.put("message", "Đã thêm nhân viên!");
            return ResponseEntity.ok(response);
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "Lỗi " + e.getMessage());
            return ResponseEntity.status(500).body(response);
        }
    }

    // ================= 3. API SỬA THÔNG TIN NHÂN VIÊN =================
    @PutMapping("/{id}")
    public ResponseEntity<Map<String, Object>> updateUser(@PathVariable Integer id, @RequestBody User updatedUser) {
        Map<String, Object> response = new HashMap<>();
        try {
            User existingUser = userRepository.findById(id).orElse(null);
            if (existingUser == null) {
                response.put("success", false);
                response.put("message", "Không tìm thấy nhân viên này!");
                return ResponseEntity.status(404).body(response);
            }
            else if(existingUser.getRole().equals("ADMIN")) {
                response.put("success",false) ;
                response.put("message","Ko được sửa") ;
                return ResponseEntity.status(400).body(response) ;
            }
            existingUser.setFullName(updatedUser.getFullName());
            existingUser.setRole(updatedUser.getRole());
            if (updatedUser.getPasswordHash() != null && !updatedUser.getPasswordHash().isEmpty()) {
                existingUser.setPasswordHash(updatedUser.getPasswordHash());
            }

            userRepository.save(existingUser);
            
            response.put("success", true);
            response.put("message", "Đã cập nhật!");
            return ResponseEntity.ok(response);
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "Lỗi server: " + e.getMessage());
            return ResponseEntity.status(500).body(response);
        }
    }

    // ================= 4. API XÓA/KHÓA NHÂN VIÊN =================
    @DeleteMapping("/{id}")
    public ResponseEntity<Map<String, Object>> deleteUser(@PathVariable Integer id) {
        Map<String, Object> response = new HashMap<>();
        User existingUser = userRepository.findById(id).orElse(null);
        if(existingUser.getRole().equals("ADMIN")) {
            response.put("success",false) ;
            response.put("message","Ko thể xóa 1 admin") ;
            return ResponseEntity.status(400).body(response) ;
        }
        try {
            userRepository.deleteById(id);
            response.put("success", true);
            response.put("message", "Đã tiễn nhân viên ra chuồng gà!");
            return ResponseEntity.ok(response);
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "Lỗi!");
            return ResponseEntity.status(500).body(response);
        }
    }
}