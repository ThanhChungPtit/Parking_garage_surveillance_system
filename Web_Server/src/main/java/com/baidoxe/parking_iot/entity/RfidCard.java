package com.baidoxe.parking_iot.entity;

import jakarta.persistence.*;
import lombok.Data;

@Entity
@Table(name = "RFID_Cards")
@Data
public class RfidCard {

    @Id
    @Column(name = "card_id", length = 50)
    private String cardId;


    @Column(name = "card_type", length = 20)
    private String cardType = "GUEST";

    @Column(name = "is_active")
    private Boolean isActive = true;
}