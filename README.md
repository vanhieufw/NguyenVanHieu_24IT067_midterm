# new_ls – Phiên bản đơn giản hóa của lệnh ls(1)

**Sinh viên:** Nguyễn Văn Hiếu – 24IT067  
**Môn học:** Lập trình Hệ thống UNIX – Bài tập giữa kỳ  
**Nền tảng mục tiêu:** NetBSD (có thể biên dịch trên hầu hết hệ thống POSIX)

Chương trình được viết từ đầu bằng C, mô phỏng một tập con các tính năng của lệnh `ls(1)` theo tài liệu man page NetBSD được cung cấp trong đề bài. Không gọi lệnh `ls` hệ thống để tạo kết quả.

**Cú pháp:**

```
./new_ls [-AacdFfhiklnqRrSstuw] [file ...]
```

- Không truyền đối số: liệt kê thư mục hiện tại.
- Truyền file: in thông tin file đó.
- Truyền thư mục: liệt kê nội dung (trừ khi dùng `-d`).
- Mặc định: một mục trên mỗi dòng.
- Mã thoát: 0 nếu thành công, khác 0 nếu có lỗi.

---

## Các tính năng đã triển khai

Hỗ trợ đầy đủ các tùy chọn trong phần SYNOPSIS của man page:

| Tùy chọn | Hành vi |
|----------|---------|
| `-A`     | Liệt kê tất cả mục trừ `.` và `..`. Tự động bật cho super-user (theo man page). |
| `-a`     | Hiển thị cả các mục bắt đầu bằng dấu chấm. |
| `-c`     | Dùng thời gian thay đổi trạng thái (ctime) khi sắp xếp (`-t`) hoặc in (`-l`). |
| `-d`     | Chỉ liệt kê bản thân thư mục, không liệt kê nội dung. Ghi đè `-R`. |
| `-F`     | Thêm ký hiệu phân loại: `/` (directory), `*` (executable), `@` (symlink), `=` (socket), `\|` (FIFO), `%` (whiteout). |
| `-f`     | Không sắp xếp. |
| `-h`     | Kích thước dễ đọc (human-readable). Ghi đè `-k`. |
| `-i`     | In số inode. |
| `-k`     | Đếm block theo kilobyte. Ghi đè `-h`. |
| `-l`     | Long format. |
| `-n`     | Long format với uid/gid dạng số. |
| `-q`     | Thay ký tự không in được bằng `?` (mặc định khi stdout là terminal). |
| `-R`     | Liệt kê đệ quy thư mục con. Ghi đè `-d`. |
| `-r`     | Đảo ngược thứ tự sắp xếp. |
| `-S`     | Sắp xếp theo kích thước (lớn trước). |
| `-s`     | Hiển thị số block đã sử dụng. |
| `-t`     | Sắp xếp theo thời gian (mới nhất trước). |
| `-u`     | Dùng thời gian truy cập (atime) khi sắp xếp (`-t`) hoặc in (`-l`). |
| `-w`     | In nguyên các ký tự không in được (mặc định khi stdout không phải terminal). |

### Hành vi bổ sung theo man page

- Tách riêng file và directory operand; file không phải directory được liệt kê trước.
- Symbolic link đưa vào dòng lệnh chỉ được follow khi nó trỏ tới directory và không dùng `-d`.
- Long format hiển thị: mode, số link, owner, group, size (hoặc major/minor với device), timestamp, tên file, và `-> target` với symbolic link.
- Số block tuân theo biến môi trường `BLOCKSIZE` (mặc định 512).
- Sticky bit, set-user-ID, set-group-ID hiển thị đúng (`t`/`T`, `s`/`S`).
- Hỗ trợ whiteout trên NetBSD (kiểu `w` và ký hiệu `%`).
- Exit status: 0 nếu thành công, >0 nếu có lỗi.

---

## Cấu trúc dự án

```
.
├── Makefile
├── README.md                 ← báo cáo này
├── .gitignore
├── .gitattributes
├── include/
│   └── new_ls.h              ← kiểu dữ liệu và khai báo dùng chung
├── src/
│   ├── main.c                ← điểm vào chương trình
│   ├── cli_parser.c          ← phân tích option + BLOCKSIZE
│   ├── file_utils.c          ← quản lý catalog động + path
│   ├── scanner.c             ← đọc directory và đệ quy
│   ├── ordering.c            ← logic sắp xếp
│   └── formatter.c           ← long format, permissions, output
└── tests/
    └── test.sh               ← bộ kiểm thử nhanh
```

---

## Yêu cầu môi trường

- NetBSD (khuyến nghị) hoặc hệ thống POSIX khác có `cc` và `make`.
- Bộ công cụ phát triển C (`cc`, header hệ thống).
- Nếu làm trên Windows: VirtualBox + NetBSD + WinSCP (SFTP).

Kiểm tra nhanh trên NetBSD:

```sh
uname -a
command -v cc
command -v make
```

Nếu thiếu `cc` hoặc `make`, cài bộ công cụ phát triển tương ứng với bản NetBSD đang dùng.

---

## Biên dịch và chạy

### Trên NetBSD (khuyến nghị)

```sh
cd ~/NguyenVanHieu_24IT067_midterm
make
make test
./new_ls -la
./new_ls -A ~
./new_ls -R .
./new_ls -St /tmp
```

Các lệnh Makefile:

| Lệnh            | Tác dụng                                      |
|-----------------|-----------------------------------------------|
| `make`          | Biên dịch chương trình `new_ls`               |
| `make test`     | Chạy bộ kiểm thử trong `tests/test.sh`        |
| `make clean`    | Xóa file object và binary                     |
| `make install`  | Cài vào `/usr/local/bin/new_ls` (cần quyền)   |
| `make uninstall`| Gỡ cài đặt                                    |

### Cài đặt tùy chọn

```sh
su
make install          # cài vào /usr/local/bin/new_ls
exit

# Hoặc không cần quyền root
make PREFIX=$HOME/.local install
export PATH="$HOME/.local/bin:$PATH"
```

Gỡ cài đặt:

```sh
make uninstall
# hoặc
make PREFIX=$HOME/.local uninstall
```

---

## Quy trình phát triển trên Windows + máy ảo NetBSD

1. Chỉnh sửa mã nguồn trên Windows bằng VS Code (hoặc editor bất kỳ).
2. Dùng WinSCP để tải các file `.c`, `.h`, `Makefile`, `README.md` lên máy ảo NetBSD.
3. Trên Terminal NetBSD chạy:

```sh
cd ~/NguyenVanHieu_24IT067_midterm
make clean
make
make test
```

4. Kiểm thử thủ công:

```sh
./new_ls -la /etc
./new_ls -R .
./new_ls -hls ~
./new_ls -F /tmp
```

**Lưu ý:** Không chỉnh sửa trực tiếp file nhị phân `new_ls` hoặc các file `.o`. Chỉ sửa source rồi `make` lại.

### Gợi ý cấu hình VirtualBox (nếu cần)

- Network Adapter: NAT + Port Forwarding cổng 22 (guest) → 2222 (host).
- Trên NetBSD bật SSH: thêm `sshd=YES` vào `/etc/rc.conf` rồi `/etc/rc.d/sshd start`.
- WinSCP kết nối: Host `127.0.0.1`, Port `2222`, protocol SFTP.

---

## Kho mã nguồn GitHub

**URL:** https://github.com/vanhieufw/NguyenVanHieu_24IT067_midterm

### Quy trình Git khuyến nghị

```sh
git status
git add .
git commit -m "Mô tả thay đổi ngắn gọn"
git push origin main
```

Không commit file nhị phân (`new_ls`), file object (`.o`) hoặc file tạm. File `.gitignore` đã cấu hình sẵn để loại trừ các file này.

---

## Giới hạn / Khác biệt đã biết

- Không triển khai multi-column layout (man page mặc định in một mục mỗi dòng).
- Tên tháng và phân loại ký tự phụ thuộc locale của hệ thống.
- Hỗ trợ whiteout chỉ có trên NetBSD; nền tảng khác sẽ bỏ qua.
- Khoảng cách cột và một số edge case rất nhỏ có thể hơi khác so với `ls` gốc của hệ thống.

---

## Tác giả

Nguyễn Văn Hiếu – MSSV 24IT067  
Bài tập giữa kỳ môn Lập trình Hệ thống UNIX – 2025/2026
