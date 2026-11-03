function fetchUsers() {
  fetch("http://localhost:8080/api/users")
    .then((res) => res.json())
    .then((data) => {
      let html = "";
      data.forEach((user) => {
        let roleColor = user.role === "ADMIN" ? "#ff4d4d" : "#00ff88";
        html += `
                    <tr>
                        <td>${user.userId || "Auto"}</td>
                        <td>${user.username}</td>
                        <td>${user.fullName || "Chưa cập nhật"}</td>
                        <td><span style="color:${roleColor}; font-weight:bold;">${user.role}</span></td>
                        <td>
                            <button class="btn-edit" onclick="editUser(${user.userId}, '${user.fullName}', '${user.role}')">Sửa</button>
                            <button class="btn-delete" onclick="deleteUser(${user.userId}, '${user.username}')">Xóa</button>
                        </td>
                    </tr>
                `;
      });
      document.getElementById("users-table").innerHTML = html;
    })
    .catch((err) => console.log("Lỗi lấy danh sách nhân viên:", err));
}

async function addNewUser() {
  let username = prompt("Nhập tên đăng nhập (VD: baove2):");
  if (!username) return;

  let password = prompt("Nhập mật khẩu (VD: 12345):");
  if (!password) return;

  let fullName = prompt("Nhập Họ và Tên:");

  let newUserData = {
    username: username,
    passwordHash: password,
    fullName: fullName,
    role: "STAFF",
  };

  try {
    const response = await fetch("http://localhost:8080/api/users", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(newUserData),
    });
    const data = await response.json();
    alert(data.message);
    if (response.ok) {
      fetchUsers();
    }
  } catch (error) {
    alert("Lỗi kết nối Backend rồi");
  }
}

async function deleteUser(id, username) {
  if (!confirm(`Có chắc chắn muốn đuổi việc nhân viên [${username}] không?`))
    return;

  try {
    const response = await fetch(`http://localhost:8080/api/users/${id}`, {
      method: "DELETE",
    });
    const data = await response.json();
    alert(data.message);
    if (response.ok) fetchUsers();
  } catch (error) {
    alert("Lỗi không kết nối được Backend!");
  }
}

async function editUser(id, oldName, oldRole) {
  let newName = prompt("Nhập Họ và Tên mới:", oldName);
  if (newName === null) return;

  let newRole = prompt("Nhập Quyền mới (ADMIN hoặc STAFF):", oldRole);
  if (newRole === null) return;

  let newPassword = prompt(
    "Nhập Mật khẩu mới (Bỏ trống nếu không muốn đổi):",
    "",
  );

  let updateData = {
    fullName: newName,
    role: newRole.toUpperCase(),
  };
  if (newPassword) updateData.passwordHash = newPassword;

  try {
    const response = await fetch(`http://localhost:8080/api/users/${id}`, {
      method: "PUT",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(updateData),
    });
    const data = await response.json();
    alert(data.message);
    if (response.ok) fetchUsers();
  } catch (error) {
    alert("Lỗi không kết nối được Backend!");
  }
}
