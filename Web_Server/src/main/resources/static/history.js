let myChart = null; // Khai báo biến biểu đồ ở đây

function fetchHistory() {
  fetch("http://localhost:8080/api/history")
    .then((res) => res.json())
    .then((data) => {
      let html = "";
      let revenueData = {};
      let reversedData = [...data].reverse();
      reversedData.forEach((row) => {
        let inTime = row.entryTime
          ? new Date(row.entryTime).toLocaleTimeString("vi-VN") +
            " " +
            new Date(row.entryTime).toLocaleDateString("vi-VN")
          : "-";
        let outTime = row.exitTime
          ? new Date(row.exitTime).toLocaleTimeString("vi-VN") +
            " " +
            new Date(row.exitTime).toLocaleDateString("vi-VN")
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
  if (myChart !== null) {
    myChart.destroy();
  }
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
