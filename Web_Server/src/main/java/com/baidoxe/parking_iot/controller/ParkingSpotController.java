package com.baidoxe.parking_iot.controller;

import com.baidoxe.parking_iot.entity.ParkingSpot;
import com.baidoxe.parking_iot.repository.ParkingSpotRepository;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/spots")
@CrossOrigin(origins = "*")
public class ParkingSpotController {

    @Autowired
    private ParkingSpotRepository parkingSpotRepository;

    public ParkingSpotController(ParkingSpotRepository parkingSpotRepository) {
        this.parkingSpotRepository = parkingSpotRepository;
    }

    @GetMapping
    public List<ParkingSpot> getAllSpots() {
        return parkingSpotRepository.findAll();
    }

    @PostMapping
    public Map<String, Object> addSpot(@RequestBody ParkingSpot newSpot) {
        Map<String, Object> response = new HashMap<>();
        try {
            newSpot.setIsOccupied(false);
            parkingSpotRepository.save(newSpot);
            response.put("success", true);
            response.put("message", "Đã thêm ô đỗ mới thành công");
            return response;
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "Lỗi server: " + e.getMessage());
            return response;
        }
    }

    @DeleteMapping("/{id}")
    public Map<String, Object> deleteSpot(@PathVariable Integer id) {
        Map<String, Object> response = new HashMap<>();
        try {
            parkingSpotRepository.deleteById(id);
            response.put("success", true);
            response.put("message", "Đã dẹp bỏ ô đỗ này thành công!");
            return response;
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "Lỗi! Đang có xe đỗ ở đây hoặc dính dữ liệu lịch sử!");
            return response;
        }
    }
}