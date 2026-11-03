function fetchRfidCards() {
  fetch("http://localhost:8080/api/rfid")
    .then((res) => res.json())
    .then((data) => {
      let html = "";
      data.forEach((card) => {
        let isLocked = card.isActive === false;
        let statusColor = isLocked ? "#ff4d4d" : "#00ff88";
        let statusText = isLocked ? "BỊ KHÓA" : "HOẠT ĐỘNG";
        let btnColor = isLocked ? "#2ecc71" : "#f39c12";
        let btnText = isLocked ? "Mở Thẻ" : "Khóa Thẻ";
        html += `
                    <tr>
                        <td><strong style="color: #f39c12;">${card.cardId}</strong></td>
                        <td><span style="color:${statusColor}; font-weight:bold;">${statusText}</span></td>
                        <td>
                            <button style="background-color: ${btnColor}; color: white; border: none; padding: 5px 10px; border-radius: 5px; cursor: pointer;" 
                                    onclick="toggleCardStatus('${card.cardId}')">
                                ${btnText}
                            </button>
                            
                            <button style="background-color: #e74c3c; color: white; border: none; padding: 5px 10px; border-radius: 5px; cursor: pointer; margin-left: 5px;" 
                                    onclick="deleteCard('${card.cardId}')">
                                Xóa
                            </button>
                        </td>
                    </tr>
                `;
      });
      document.getElementById("rfid-table").innerHTML = html;
    })
    .catch((err) => console.log("Lỗi lấy danh sách thẻ:", err));
}

async function deleteCard(cardId) {
  if (!confirm(`Xác nhận XÓA VĨNH VIỄN thẻ [${cardId}] khỏi hệ thống?`)) {
    return;
  }

  try {
    const response = await fetch(`http://localhost:8080/api/rfid/${cardId}`, {
      method: "DELETE",
    });
    const data = await response.json();
    if (data.success) {
      alert("✅ " + data.message);
      fetchRfidCards();
    } else {
      alert("❌ Lỗi từ Server: " + data.message);
    }
  } catch (error) {
    alert("Mất kết nối với Backend!");
  }
}

async function toggleCardStatus(cardId) {
  if (!confirm(`Xác nhận thay đổi trạng thái của thẻ [${cardId}]?`)) {
    return;
  }

  try {
    const response = await fetch(
      `http://localhost:8080/api/rfid/toggle/${cardId}`,
      {
        method: "PUT",
      },
    );
    const data = await response.json();
    if (data.success) {
      fetchRfidCards();
    } else {
      alert("Lỗi: " + data.message);
    }
  } catch (error) {
    alert("Mất kết nối với Server!");
  }
}

async function addNewRfidCard() {
  let uid = prompt("Nhập mã Thẻ (VD: CARD_VIP_02):");
  if (!uid) return;

  let newCardData = {
    cardId: uid,
    status: "ACTIVE",
  };

  try {
    const response = await fetch("http://localhost:8080/api/rfid", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(newCardData),
    });
    const data = await response.json();
    alert(data.message);
    if (response.ok) fetchRfidCards();
  } catch (error) {
    alert("Lỗi không kết nối được Backend!");
  }
}
