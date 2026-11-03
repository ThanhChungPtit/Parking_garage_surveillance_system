async function toggleBarrier(gate, btnElement) {
  const isEntrance = gate === "in";
  const gateCode = isEntrance ? "1" : "2";
  const currentText = btnElement.innerText;
  let command = "";
  let nextText = "";
  let nextColor = "";
  if (currentText.includes("Mở")) {
    command = "O" + gateCode;
    nextText = `Đóng Barrie đường ${isEntrance ? "vào" : "ra"}`;
    nextColor = "#2c3e50";
  } else {
    command = "C" + gateCode;
    nextText = `Mở Barrie đường ${isEntrance ? "vào" : "ra"}`;
    nextColor = isEntrance ? "#e67e22" : "#e74c3c";
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
      console.log(
        `Đã gửi thành công lệnh ${command} tới /ptit/servo/emergency`,
      );
    } else {
      alert("Lỗi: " + data.message);
    }
  } catch (error) {
    alert("Mất kết nối với Backend!");
  }
}
