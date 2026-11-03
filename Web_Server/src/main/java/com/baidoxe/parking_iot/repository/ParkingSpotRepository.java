package com.baidoxe.parking_iot.repository;

import com.baidoxe.parking_iot.entity.ParkingSpot;
import org.springframework.stereotype.Repository;

import javax.sql.DataSource;
import java.sql.*;
import java.util.ArrayList;
import java.util.List;

@Repository
public class ParkingSpotRepository {

    // Nhờ Spring cấp cho thông tin Database (URL, username, password) đã cấu hình
    private final DataSource dataSource;

    public ParkingSpotRepository(DataSource dataSource) {
        this.dataSource = dataSource;
    }

    // =====================================================================
    // 1. Dùng Java Thuần đọc toàn bộ dữ liệu (SELECT)
    // =====================================================================
    public List<ParkingSpot> findAll() {
        List<ParkingSpot> spots = new ArrayList<>();
        String sql = "SELECT * FROM parking_spots";

        // Mở kết nối (Connection) và Tạo câu lệnh (Statement)
        try (Connection conn = dataSource.getConnection();
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
             
            // ResultSet là bảng dữ liệu Database trả về. Dùng vòng lặp while để đọc từng dòng.
            while (rs.next()) {
                ParkingSpot spot = new ParkingSpot();
                spot.setSpotId(rs.getInt("spot_id"));
                spot.setSpotName(rs.getString("spot_name"));
                spot.setSensorId(rs.getString("sensor_id"));
                spot.setIsOccupied(rs.getBoolean("is_occupied"));
                
                spots.add(spot); // Thêm vào danh sách List
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return spots;
    }

    // =====================================================================
    // 2. Dùng Java Thuần để thêm mới hoặc cập nhật (INSERT / UPDATE)
    // =====================================================================
    public ParkingSpot save(ParkingSpot spot) {
        // 1. Kiểm tra rỗng (Validation)
        if (spot.getSpotName() == null || spot.getSpotName().trim().isEmpty()) {
            throw new IllegalArgumentException("Tên ô đỗ không được bỏ trống!");
        }
        if (spot.getSensorId() == null || spot.getSensorId().trim().isEmpty()) {
            throw new IllegalArgumentException("Mã cảm biến không được bỏ trống!");
        }
        
        // 2. Mặc định isOccupied = false nếu không có dữ liệu
        if (spot.getIsOccupied() == null) {
            spot.setIsOccupied(false);
        }

        try (Connection conn = dataSource.getConnection()) {
            // 3. Kiểm tra trùng lặp Tên ô đỗ
            String checkNameSql = "SELECT spot_id FROM parking_spots WHERE spot_name = ?";
            try (PreparedStatement pstmt = conn.prepareStatement(checkNameSql)) {
                pstmt.setString(1, spot.getSpotName().trim());
                try (ResultSet rs = pstmt.executeQuery()) {
                    // Nếu tìm thấy tên này trong DB, VÀ (đang tạo mới HOẶC id tìm được khác id đang sửa) -> Trùng!
                    if (rs.next() && (spot.getSpotId() == null || rs.getInt("spot_id") != spot.getSpotId())) {
                        throw new IllegalArgumentException("Tên ô đỗ '" + spot.getSpotName() + "' đã tồn tại!");
                    }
                }
            }

            // 4. Kiểm tra trùng lặp Mã cảm biến
            String checkSensorSql = "SELECT spot_id FROM parking_spots WHERE sensor_id = ?";
            try (PreparedStatement pstmt = conn.prepareStatement(checkSensorSql)) {
                pstmt.setString(1, spot.getSensorId().trim());
                try (ResultSet rs = pstmt.executeQuery()) {
                    if (rs.next() && (spot.getSpotId() == null || rs.getInt("spot_id") != spot.getSpotId())) {
                        throw new IllegalArgumentException("Mã cảm biến '" + spot.getSensorId() + "' đã được gắn cho ô khác!");
                    }
                }
            }

            // 5. Nếu vượt qua hết bài kiểm tra thì mới cho INSERT / UPDATE
            if (spot.getSpotId() == null) {
                // Chưa có ID -> Lệnh INSERT
                String sql = "INSERT INTO parking_spots (spot_name, sensor_id, is_occupied) VALUES (?, ?, ?)";
                try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                    pstmt.setString(1, spot.getSpotName());
                    pstmt.setString(2, spot.getSensorId());
                    pstmt.setBoolean(3, spot.getIsOccupied());
                    pstmt.executeUpdate();
                }
            } else {
                // Đã có ID -> Lệnh UPDATE
                String sql = "UPDATE parking_spots SET spot_name=?, sensor_id=?, is_occupied=? WHERE spot_id=?";
                try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                    pstmt.setString(1, spot.getSpotName());
                    pstmt.setString(2, spot.getSensorId());
                    pstmt.setBoolean(3, spot.getIsOccupied());
                    pstmt.setInt(4, spot.getSpotId());
                    pstmt.executeUpdate();
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
            throw new RuntimeException("Lỗi Database: " + e.getMessage());
        }
        return spot;
    }

    // =====================================================================
    // 3. Dùng Java Thuần để Xóa dữ liệu (DELETE)
    // =====================================================================
    public void deleteById(Integer id) {
        String sql = "DELETE FROM parking_spots WHERE spot_id = ?";
        try (Connection conn = dataSource.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
             
            pstmt.setInt(1, id); // Nhét số ID vào chỗ dấu "?"
            pstmt.executeUpdate(); // Bấm nút "Chạy lệnh"
            
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    // =====================================================================
    // 4. Dùng Java Thuần tìm 1 ô đỗ theo mã Cảm biến (SELECT ... WHERE)
    // =====================================================================
    public ParkingSpot findBySensorId(String sensorId) {
        String sql = "SELECT * FROM parking_spots WHERE sensor_id = ?";
        try (Connection conn = dataSource.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
             
            pstmt.setString(1, sensorId);
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    ParkingSpot spot = new ParkingSpot();
                    spot.setSpotId(rs.getInt("spot_id"));
                    spot.setSpotName(rs.getString("spot_name"));
                    spot.setSensorId(rs.getString("sensor_id"));
                    spot.setIsOccupied(rs.getBoolean("is_occupied"));
                    return spot;
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return null;
    }
}