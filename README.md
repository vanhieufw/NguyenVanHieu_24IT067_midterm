# new_ls – Phiên bản đơn giản hóa của lệnh `ls(1)` trên NetBSD

**Sinh viên:** Nguyễn Văn Hiếu – 24IT067  
**Môn học:** Lập trình Hệ thống UNIX – Bài tập giữa kỳ  
**Nền tảng:** NetBSD (có thể biên dịch trên các hệ thống POSIX khác)

Đây là phiên bản tự phát triển từ đầu, mô phỏng một tập con các tính năng của lệnh `ls(1)` theo sổ tay NetBSD được cung cấp trong đề bài.

---

## Các tính năng đã triển khai

Hỗ trợ đầy đủ tất cả các tùy chọn trong phần SYNOPSIS của man page:

```
new_ls [-AacdFfhiklnqRrSstuw] [file ...]
```

| Tùy chọn | Hành vi |
|----------|---------|
| `-A`     | Liệt kê tất cả các mục trừ `.` và `..`. **Tự động bật cho super-user** (theo đúng man page). |
| `-a`     | Hiển thị cả các mục có tên bắt đầu bằng dấu chấm (`.`). |
| `-c`     | Sử dụng thời gian thay đổi trạng thái (ctime) khi sắp xếp (`-t`) hoặc in (`-l`). |
| `-d`     | Chỉ liệt kê bản thân thư mục, không liệt kê nội dung. Ghi đè `-R`. |
| `-F`     | Thêm ký hiệu phân loại: `/` directory, `*` executable, `@` symbolic link, `=` socket, `\|` FIFO, `%` whiteout. |
| `-f`     | Không sắp xếp. |
| `-h`     | Hiển thị kích thước human-readable (ghi đè `-k`). |
| `-i`     | In số inode. |
| `-k`     | Đếm block theo kilobyte (ghi đè `-h`). |
| `-l`     | Long format listing. |
| `-n`     | Long format với uid/gid dạng số. |
| `-q`     | In ký tự không in được thành `?` (mặc định khi stdout là terminal). |
| `-R`     | Liệt kê đệ quy các thư mục con. Ghi đè `-d`. |
| `-r`     | Đảo ngược thứ tự sắp xếp. |
| `-S`     | Sắp xếp theo size (lớn trước). |
| `-s`     | Hiển thị số block được sử dụng. |
| `-t`     | Sắp xếp theo time (mới nhất trước). |
| `-u`     | Sử dụng thời gian truy cập (atime) khi sắp xếp (`-t`) hoặc in (`-l`). |
| `-w`     | In thô các ký tự không in được (mặc định khi stdout không phải terminal). |

### Các hành vi bổ sung theo man page

- Các operand file và directory được tách riêng; file không phải directory được liệt kê trước.
- Symbolic link được đưa vào dòng lệnh sẽ được follow chỉ khi nó trỏ tới directory và không dùng `-d`.
- Long format hiển thị: mode, số link, owner, group, size (hoặc major/minor với device), timestamp, tên file, và `-> target` với symbolic link.
- Số block tuân theo biến môi trường `BLOCKSIZE` (mặc định 512).
- Sticky bit, set-user-ID và set-group-ID được hiển thị đúng (`t`/`T`, `s`/`S`).
- Hỗ trợ whiteout trên NetBSD (kiểu `w` và ký hiệu `%`).
- Exit status: 0 nếu thành công, >0 nếu có lỗi.

---

## Cấu trúc dự án

```
.
├── Makefile
├── README.md                 ← báo cáo này
├── .gitignore
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
```

---

## Quy trình phát triển trên Windows + máy ảo NetBSD

1. Chỉnh sửa mã nguồn trên Windows bằng VS Code.
2. Dùng WinSCP để tải các file `.c`, `.h`, `README.md` đã sửa lên máy ảo NetBSD.
3. Trên NetBSD chạy:
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
   ```

**Không** chỉnh sửa trực tiếp file nhị phân `new_ls` hoặc các file `.o`.

---

## Kho mã nguồn GitHub

**URL:** https://github.com/YOUR_USERNAME/NguyenVanHieu_24IT067_midterm

(Thay `YOUR_USERNAME` bằng tài khoản GitHub của bạn.)

### Quy trình Git khuyến nghị

```sh
git status
git add .
git commit -m "Fix: enable -A for super-user, whiteout support, setlocale and rewrite README"
git push origin main
```

---

## Giới hạn / Khác biệt đã biết

- Không triển khai multi-column interactive layout (man page mặc định in một mục mỗi dòng).
- Tên tháng và phân loại ký tự phụ thuộc vào locale của hệ thống.
- Hỗ trợ whiteout chỉ có trên NetBSD; các nền tảng khác sẽ bỏ qua.
- Khoảng cách chính xác và một số edge case có thể hơi khác so với lệnh `ls` gốc của hệ thống.

---

## Tác giả

Nguyễn Văn Hiếu – MSSV 24IT067  
Bài tập giữa kỳ môn Lập trình Hệ thống UNIX – 2025/2026
