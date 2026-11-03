function fetchManageSpots() {
  fetch("http://localhost:8080/api/spots")
    .then((res) => res.json())
    .then((data) => {
      let html = "";
      data.forEach((spot) => {
        let statusText = spot.isOccupied
          ? '<span style="color:#ff4d4d; font-weight:bold;">CÓ XE</span>'
          : '<span style="color:#00ff88; font-weight:bold;">TRỐNG</span>';
        let currentId = spot.spotId || spot.id;
        let sensorDisplay = spot.sensorId
          ? `<strong style="color:#f39c12">${spot.sensorId}</strong>`
          : '<i style="color:gray">Chưa gắn</i>';
        html += `
                    <tr>
                        <td>${currentId}</td>
                        <td><strong style="font-size: 16px;">${spot.spotName}</strong></td>
                        <td>${sensorDisplay}</td>
                        <td>${statusText}</td>
                        <td>
                            <button class="btn-delete" onclick="deleteSpot(${currentId}, '${spot.spotName}')">Xóa Ô Này</button>
                        </td>
                    </tr>
                `;
      });
      document.getElementById("manage-spots-table").innerHTML = html;
    })
    .catch((err) => console.log("Lỗi lấy bãi đỗ:", err));
}

async function addNewSpot() {
  let spotName = prompt("Nhập tên ô đỗ mới (VD: Ô số 5):");
  if (!spotName) return;

  let sensorIdInput = prompt(
    "Nhập mã Cảm Biến gắn ở ô này (VD: S5).\nLưu ý: Không được nhập trùng với ô khác!",
  );

  let newSpotData = {
    spotName: spotName,
    isOccupied: false,
    sensorId: sensorIdInput || null,
  };

  try {
    const response = await fetch("http://localhost:8080/api/spots", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(newSpotData),
    });
    const data = await response.json();
    alert(data.message || "Thành công!");
    if (response.ok) {
      fetchManageSpots();
      fetchSpots();
    }
  } catch (error) {
    alert("Lỗi kết nối Backend!");
  }
}

async function deleteSpot(id, name) {
  if (!confirm(`Có chắc chắn muốn đập bỏ [${name}] không?`)) return;
  try {
    const response = await fetch(`http://localhost:8080/api/spots/${id}`, {
      method: "DELETE",
    });
    const data = await response.json();
    alert(data.message || "Đã xóa!");
    if (response.ok) {
      fetchManageSpots();
      fetchSpots();
    }
  } catch (error) {
    alert("Lỗi kết nối Backend!");
  }
}
