function checkAuth() {
  const token = localStorage.getItem("iot_parking_token");
  const role = localStorage.getItem("iot_parking_role");
  if (token) {
    document.getElementById("login-container").style.display = "none";
    document.getElementById("app-container").style.display = "flex";
    document.getElementById("welcome-text").innerText = `Xin chào, ${role}`;
    document.getElementById("tab-dashboard").classList.add("active");

    if (role === "ADMIN") {
      document.getElementById("sidebar").style.display = "block";
    } else {
      document.getElementById("sidebar").style.display = "none";
    }
    fetchSpots();
  } else {
    document.getElementById("login-container").style.display = "flex";
    document.getElementById("app-container").style.display = "none";
  }
}

async function handleLogin() {
  const user = document.getElementById("username").value;
  const pass = document.getElementById("password").value;
  const errorMsg = document.getElementById("login-error");

  try {
    const response = await fetch("http://localhost:8080/api/auth/login", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ username: user, password: pass }),
    });
    const data = await response.json();
    if (response.ok) {
      localStorage.setItem("iot_parking_token", data.token);
      localStorage.setItem("iot_parking_role", data.role);
      errorMsg.style.display = "none";
      checkAuth();
    } else {
      errorMsg.style.display = "block";
      errorMsg.innerText = data.message || "Sai tài khoản hoặc mật khẩu!";
    }
  } catch (error) {
    errorMsg.style.display = "block";
    errorMsg.innerText = "Không kết nối được Server!";
  }
}

function handleLogout() {
  localStorage.clear();
  document.getElementById("username").value = "";
  document.getElementById("password").value = "";
  checkAuth();
}
