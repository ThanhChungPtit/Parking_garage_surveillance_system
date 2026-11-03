package com.baidoxe.parking_iot.repository;

import com.baidoxe.parking_iot.entity.RfidCard;
import java.sql.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import org.springframework.stereotype.Repository;

@Repository
public class RfidCardRepository {
    private static final String URL = "jdbc:mysql://localhost:3306/baidoxe?useSSL=false&serverTimezone=Asia/Ho_Chi_Minh&characterEncoding=UTF-8&allowPublicKeyRetrieval=true";
    private static final String USERNAME = "root";
    private static final String PASSWORD = "Chung2004@";

    public List<RfidCard> findAll() {
        List<RfidCard> cards = new ArrayList<>() ;
        String cmd = "SELECT * FROM RFID_Cards" ;
        try (Connection connect = DriverManager.getConnection(URL,USERNAME,PASSWORD) ;
                Statement stm=connect.createStatement() ;
                ResultSet result=stm.executeQuery(cmd)) {
                    while(result.next()) {
                        RfidCard card = new RfidCard() ;
                        card.setCardId(result.getString("card_id"));
                        card.setCardType(result.getString("card_type"));
                        card.setIsActive(result.getBoolean("is_active")); 
                        cards.add(card) ;
                    }
                } catch (SQLException e) {
                    e.printStackTrace();
                }
                return cards ;
                }
    public RfidCard save(RfidCard card) {
        String cmd = "INSERT INTO RFID_Cards (card_id, card_type, is_active) VALUES (?,?,?)" ;
        try (Connection connect = DriverManager.getConnection(URL,USERNAME,PASSWORD) ;
                PreparedStatement stm=connect.prepareStatement(cmd)) 
                {
                    stm.setString(1, card.getCardId()) ;
                    stm.setString(2, card.getCardType()) ;
                    stm.setBoolean(3, card.getIsActive()); 
                    stm.executeUpdate() ;//trả về số dòng bị ảnh hưởng
                    
                }
                catch (SQLException e) {
                    e.printStackTrace();
                }
                return card ;

    }
    public void deleteById(String cardID) {
        String cmd = "DELETE FROM RFID_Cards WHERE card_id = ?" ;
        try (Connection connect = DriverManager.getConnection(URL,USERNAME,PASSWORD) ;
                PreparedStatement stm=connect.prepareStatement(cmd))
                {
                    stm.setString(1, cardID);
                    stm.executeUpdate() ;

                }
                catch (SQLException e) {
                    e.printStackTrace();
                }
    }
    public Optional<RfidCard> findById(String cardID) {
        String cmd = "SELECT* FROM RFID_Cards WHERE card_id = ?" ;
        try (Connection connect = DriverManager.getConnection(URL,USERNAME,PASSWORD) ;
                PreparedStatement stm = connect.prepareStatement(cmd)) {
                    stm.setString(1, cardID) ;
                    try (ResultSet result = stm.executeQuery()) {
                        if(result.next()) {
                            RfidCard card = new RfidCard() ;
                            card.setCardId(result.getString("card_id"));  
                            card.setCardType(result.getString("card_type"));
                            card.setIsActive(result.getBoolean("is_active"));
                            return Optional.of(card) ;

                        }
                    }
                }
                catch (SQLException e) {
                    e.printStackTrace();
                }
                return Optional.empty() ;
    }
    public void setActive(String cardID, boolean isActive) {
        String cmd = "UPDATE RFID_Cards SET is_active = ? WHERE card_id = ?" ;
        try (Connection connect = DriverManager.getConnection(URL,USERNAME,PASSWORD) ;
                PreparedStatement stm = connect.prepareStatement(cmd)) {
                    stm.setBoolean(1, isActive); 
                    stm.setString(2,cardID) ;
                    stm.executeUpdate() ;
                }
                catch (SQLException e) {
                    e.printStackTrace();
                }
    }
    }




