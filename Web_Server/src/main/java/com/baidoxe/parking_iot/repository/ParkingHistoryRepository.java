package com.baidoxe.parking_iot.repository;

import com.baidoxe.parking_iot.entity.ParkingHistory;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface ParkingHistoryRepository extends JpaRepository<ParkingHistory, Integer> {
    // Hàm này để lúc xe ra quẹt thẻ, mình tìm xem cái phiên đỗ xe ĐANG CHẠY của cái thẻ đấy là phiên nào để còn chốt giờ ra tính tiền
    ParkingHistory findByRfidCard_CardIdAndStatus(String cardId, String status);
}