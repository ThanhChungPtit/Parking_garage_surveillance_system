// ================= LOGIC BÃI XE, LỊCH SỬ & BARRIE =================
function fetchSpots() {
  if (!sessionStorage.getItem("iot_parking_token")) return;
  fetch("http://localhost:8080/api/spots")
    .then((res) => res.json())
    .then((data) => {
      let html = "";
      data.forEach((spot) => {
        html += `<div class="spot ${spot.isOccupied ? "full" : "empty"}">
                        <div>${spot.spotName}</div>
                        <div style="font-size: 14px; margin-top: 10px;">${spot.isOccupied ? "CÓ XE" : "TRỐNG"}</div>
                    </div>`;
      });
      document.getElementById("parking-lot").innerHTML = html;
    })
    .catch((err) => console.log(err));
}

let myChart = null;

async function simulateScan() {
  let cardId = prompt("Hãy nhập mã thẻ RFID để quẹt:");
  if (!cardId) return;
  try {
    const response = await fetch(
      `http://localhost:8080/api/rfid/scan/${cardId}`,
      { method: "POST" },
    );
    const data = await response.json();
    alert(data.message);
    fetchHistory();
    fetchSpots();
  } catch (error) {
    alert("Lỗi không kết nối được Backend lúc quẹt thẻ!");
  }
}

function fetchHistory() {
  fetch("http://localhost:8080/api/history")
    .then((res) => res.json())
    .then((data) => {
      let html = "";
      let revenueData = {};
      let reversedData = [...data].reverse();
      reversedData.forEach((row) => {
        let inTime = row.entryTime
          ? new Date(row.entryTime).toLocaleTimeString("vi-VN")
          : "-";
        let outTime = row.exitTime
          ? new Date(row.exitTime).toLocaleTimeString("vi-VN")
          : "-";
        let statusHtml =
          row.status === "IN"
            ? '<span style="color:#00ff88;">Đang gửi</span>'
            : '<span style="color:#ff4444;">Đã ra</span>';
        let fee = row.fee ? row.fee + " đ" : "-";
        let cardDisplay =
          row.rfidCard && row.rfidCard.cardId
            ? row.rfidCard.cardId
            : "Khách lạ";
        html += `<tr><td><strong style="color: #00d2ff;">${cardDisplay}</strong></td><td>${inTime}</td><td>${outTime}</td><td>${statusHtml}</td><td>${fee}</td></tr>`;
        if (row.status === "OUT" && row.fee) {
          let dateKey = new Date(row.exitTime).toLocaleDateString("vi-VN");
          if (revenueData[dateKey]) {
            revenueData[dateKey] += parseFloat(row.fee);
          } else {
            revenueData[dateKey] = parseFloat(row.fee);
          }
        }
      });
      document.getElementById("history-table").innerHTML = html;
      drawChart(Object.keys(revenueData), Object.values(revenueData));
    })
    .catch((err) => console.log("Lỗi kéo lịch sử:", err));
}

function drawChart(labels, dataPoints) {
  const canvas = document.getElementById("revenueChart");
  if (!canvas) return;
  const ctx = canvas.getContext("2d");
  if (myChart != null) myChart.destroy();
  myChart = new Chart(ctx, {
    type: "bar",
    data: {
      labels: labels,
      datasets: [
        {
          label: "Doanh thu (VNĐ)",
          data: dataPoints,
          backgroundColor: "rgba(243, 156, 18, 0.8)",
          borderColor: "rgba(243, 156, 18, 1)",
          borderWidth: 1,
          borderRadius: 5,
        },
      ],
    },
    options: {
      scales: {
        y: { beginAtZero: true, ticks: { color: "white" } },
        x: { ticks: { color: "white" } },
      },
      plugins: { legend: { labels: { color: "white" } } },
    },
  });
}

async function toggleBarrier(gate, btnElement) {
  const isEntrance = gate === "in";
  const gateCode = isEntrance ? "1" : "2";
  const currentText = btnElement.innerText;
  let command = "";
  let nextText = "";
  let nextColor = "";
  // Thay đổi dòng 94
  if (currentText.startsWith("Mở")) {
    command = "O" + gateCode;
    nextText = `Đóng Barrie đường ${isEntrance ? "vào" : "ra"}`;
    nextColor = "#2c3e50";
  } else {
    command = "C" + gateCode;
    nextText = `Mở Barrie đường ${isEntrance ? "vào" : "ra"}`;
    nextColor = isEntrance ? "#ff9f43" : "#ee5253";
  }
  try {
    const response = await fetch(
      `http://localhost:8080/api/barrier/emergency/${command}`,
      { method: "POST" },
    );
    const data = await response.json();
    if (data.success) {
      btnElement.innerText = nextText;
      btnElement.style.backgroundColor = nextColor;
    } else alert("Lỗi: " + data.message);
  } catch (error) {
    alert("Mất kết nối với Backend!");
  }
}
