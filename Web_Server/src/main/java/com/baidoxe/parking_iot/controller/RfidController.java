package com.baidoxe.parking_iot.controller;

import com.baidoxe.parking_iot.entity.ParkingHistory;
import com.baidoxe.parking_iot.entity.ParkingSpot;
import com.baidoxe.parking_iot.entity.RfidCard;
import com.baidoxe.parking_iot.repository.ParkingHistoryRepository;
import com.baidoxe.parking_iot.repository.ParkingSpotRepository;
import com.baidoxe.parking_iot.repository.RfidCardRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;


@RestController
@RequestMapping("/api/rfid")
@CrossOrigin(origins = "*")
public class RfidController {

    @Autowired
    private RfidCardRepository cardRepo;

    @Autowired
    private ParkingHistoryRepository historyRepo;

    @Autowired
    private ParkingSpotRepository spotRepo; 

    @PostMapping("/scan/{cardId}")
    public Map<String, Object> scanCard(@PathVariable String cardId) {
        Map<String, Object> response = new HashMap<>();
        Optional<RfidCard> cardOpt = cardRepo.findById(cardId);
        if (cardOpt.isEmpty()) {
            response.put("success", false);
            response.put("message", "Thẻ giả hoặc chưa đăng ký!");
            return response;
        }
        RfidCard card = cardOpt.get() ;
        if(card.getIsActive() != null && !card.getIsActive()) {
            response.put("success", false) ;
            response.put("message","The bi Lock") ;
            return response ;
        }

        ParkingHistory activeSession = historyRepo.findByRfidCard_CardIdAndStatus(cardId, "IN");

        if (activeSession == null) {
            // ================= XE VÀO BÃI =================
            
            // 2. TÌM Ô TRỐNG
            List<ParkingSpot> allSpots = spotRepo.findAll();
            ParkingSpot availableSpot = null;
            for (ParkingSpot spot : allSpots) {
                if (!spot.getIsOccupied()) { // Nếu thấy ô nào isOccupied = false
                    availableSpot = spot;
                    break; // Lấy luôn ô đầu tiên tìm thấy
                }
            }

            // Xử lý logic nếu bãi đỗ xe đã đầy
            if (availableSpot == null) {
                response.put("success", false);
                response.put("message", "Bãi xe đã ĐẦY! Không thể mở Barie.");
                return response;
            }

            // 3. GHI GIỜ VÀO (ENTRY TIME)
            ParkingHistory newSession = new ParkingHistory();
            newSession.setRfidCard(cardOpt.get());
            newSession.setEntryTime(LocalDateTime.now());
            newSession.setStatus("IN");
            
            // Nếu Database của sếp có liên kết khóa ngoại spot_id trong bảng lịch sử, 
            // sếp có thể mở comment dòng dưới này:
            // newSession.setSpotId(availableSpot.getSpotId()); 
            
            historyRepo.save(newSession);
            
            // 4. BÁO THIẾT BỊ MỞ BARRIER
            response.put("success", true);
            response.put("action", "ENTRY");
            response.put("message", "Mở barie! Mời đỗ xe vào: " + availableSpot.getSpotName());

        } else {
            // ================= XE RA KHỎI BÃI =================
            
            // 1. TÌM LỊCH SỬ GIỜ VÀO
            LocalDateTime now = LocalDateTime.now();
            activeSession.setExitTime(now); // Cập nhật giờ ra
            
            // 2. TRỪ ĐI GIỜ RA ĐỂ TÍNH PHÍ
            java.time.LocalDate entryDate = activeSession.getEntryTime().toLocalDate();
            java.time.LocalDate exitDate = now.toLocalDate();
            long daysBetween = java.time.temporal.ChronoUnit.DAYS.between(entryDate, exitDate);
            
            double fee = (daysBetween == 0) ? 5000.0 : 10000.0;
            activeSession.setFee(fee); 
            
            // 3. CẬP NHẬT TRẠNG THÁI "ĐÃ RA"
            activeSession.setStatus("OUT");
            historyRepo.save(activeSession);
            
            // 4. BÁO MỞ BARRIER
            response.put("success", true);
            response.put("action", "EXIT");
            response.put("message", "Mở barie! Thu phí: " + fee + " đ. Tạm biệt!");
        }
        return response;
    }

    @GetMapping
    public List<RfidCard> getAllCards() {
        return cardRepo.findAll();
    }

    @PostMapping
    public Map<String, Object> addCard(@RequestBody RfidCard newCard) {
        Map<String, Object> response = new HashMap<>();
        try {
            cardRepo.save(newCard);
            response.put("success", true);
            response.put("message", "Thêm thẻ thành công!");
        } catch (Exception e) {
            response.put("success", false);
            response.put("message", "Lỗi: Thẻ này có thể đã tồn tại!");
        }
        return response;
    }


    @DeleteMapping("/{cardId}")
    public Map<String, Object> deleteCard(@PathVariable String cardId) {
        Map<String, Object> response = new HashMap<>();
        try {
            cardRepo.deleteById(cardId);
            response.put("success", true);
        } catch (Exception e) {
            response.put("success", false);
        }
        return response;
    }


   @PutMapping("/toggle/{cardID}")
   public Map<String, Object> DaoStatusCard(@PathVariable String cardID) {
        Map<String, Object> response = new HashMap<>() ;
        Optional<RfidCard> cardO= cardRepo.findById(cardID) ;
        if(cardO.isEmpty()) {
            response.put("success", false) ;
            response.put("message","Không tìm thấy mã thẻ này!") ;
        }
        else {
            RfidCard card = cardO.get() ;
            boolean isActive = !card.getIsActive() ;
            cardRepo.setActive(cardID, isActive);
            response.put("success",true) ;
            response.put("message", isActive ? "Đã mở khóa thẻ!" : "Đã khóa thẻ!");
            response.put("currentStatus", isActive) ;
        }
        return response ;
   }
}