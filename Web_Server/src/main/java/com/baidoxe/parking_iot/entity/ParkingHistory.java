package com.baidoxe.parking_iot.entity;

import jakarta.persistence.*;
import lombok.Data;
import java.time.LocalDateTime;

@Entity
@Table(name = "Parking_History")
@Data
public class ParkingHistory {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    @Column(name = "session_id")
    private Integer sessionId;

    // Nối với bảng RfidCard (Many-to-One)
    @ManyToOne
    @JoinColumn(name = "card_id")
    private RfidCard rfidCard;

    // Nối với bảng ParkingSpot (Many-to-One)
    @ManyToOne
    @JoinColumn(name = "spot_id")
    private ParkingSpot parkingSpot;

    @Column(name = "entry_time")
    private LocalDateTime entryTime = LocalDateTime.now();

    @Column(name = "exit_time")
    private LocalDateTime exitTime;

    @Column(name = "fee")
    private Double fee = 0.0;

    @Column(length = 20)
    private String status = "PARKING";
}