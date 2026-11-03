// app.js giữ bootstrap và điều phối chung cho trang
function switchTab(tabId, menuItem) {
  let tabs = document.querySelectorAll(".tab-content");
  tabs.forEach((tab) => tab.classList.remove("active"));

  let items = document.querySelectorAll(".menu-item");
  items.forEach((item) => item.classList.remove("active"));

  document.getElementById(tabId).classList.add("active");
  menuItem.classList.add("active");

  if (tabId === "tab-users") fetchUsers();
  if (tabId === "tab-rfid") fetchRfidCards();
  if (tabId === "tab-spots") fetchManageSpots();
  if (tabId === "tab-dashboard") {
    fetchSpots();
    fetchHistory();
  }
}

checkAuth();
setInterval(fetchHistory, 2000);
