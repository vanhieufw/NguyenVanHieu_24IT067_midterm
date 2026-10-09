````markdown
<div align="center">

# 🗃️ new_lss

**A modular UNIX file-listing utility in C**

![Language](https://img.shields.io/badge/C-C11-00599C?logo=c&logoColor=white) ![Target](https://img.shields.io/badge/Target-NetBSD-EAB92D) ![Build](https://img.shields.io/badge/Build-BSD%20make-blue) ![Options](https://img.shields.io/badge/Flags-19-brightgreen)

</div>

## Overview

`new_ls` là một phiên bản tự phát triển từ đầu nhằm mục đích học tập, mô phỏng lại một phần tập lệnh trong sổ tay NetBSD `ls(1)` được phân phối cho bài thi giữa kỳ môn Lập trình Hệ thống UNIX. Chương trình không bao giờ gọi lệnh `ls` của hệ thống để tạo đầu ra. Mã nguồn được chia thành 6 mô-đun xử lý riêng biệt cùng một giao diện dùng chung (`include/new_ls.h`).

## Requirements

- NetBSD cùng các công cụ phát triển (`cc`, `make`), hoặc một môi trường POSIX tương thích khác.
- Không yêu cầu thư viện runtime bên thứ ba nào.
- Git (tùy chọn) để clone repository.

## Build and test

```sh
make
make test
./new_ls -la
```
````

## Optional installation (run without `./`)

Chạy `make install` với đủ quyền hạn; vị trí mặc định là `/usr/local/bin/new_ls`.

```sh
make
su
make install
exit
new_ls -la

```

Nếu gặp lỗi `new_ls: not found`, hãy kiểm tra `/usr/local/bin` đã có trong `PATH` chưa. Đối với Bash, chạy `export PATH="/usr/local/bin:$PATH"` trong shell hiện tại. Để cài đặt không cần quyền root vào prefix cá nhân, chạy `make PREFIX="$HOME/.local" install` và thêm `$HOME/.local/bin` vào `PATH`.

Để gỡ bỏ file binary đã cài đặt, sử dụng `make uninstall` với cùng quyền hạn và prefix. **File `/bin/ls` của hệ thống sẽ không bao giờ bị ghi đè.**

## Usage

```text
new_ls [-AacdFfhiklnqRrSstuw] [file ...]

```

| Option                 | Behavior                                                           |
| ---------------------- | ------------------------------------------------------------------ |
| `-a`, `-A`             | Hiển thị dotfiles; `-A` loại trừ `.` và `..`                       |
| `-c`, `-u`             | Chọn thời gian status-change hoặc thời gian truy cập (access time) |
| `-d`, `-R`             | Liệt kê bản thân các thư mục hoặc duyệt đệ quy (recurse)           |
| `-F`                   | Thêm các ký hiệu chỉ thị kiểu file (type indicators)               |
| `-f`, `-r`, `-S`, `-t` | Không sắp xếp, đảo ngược, sắp xếp theo size hoặc time              |
| `-h`, `-k`, `-s`       | Dung lượng dễ đọc (human sizes), đơn vị block 1-KiB, số block      |
| `-i`                   | Hiển thị số inode                                                  |
| `-l`, `-n`             | Liệt kê chi tiết (long listing), hiển thị user/group dạng số       |
| `-q`, `-w`             | Thay thế các ký tự không in được hoặc in tên dạng thô (raw)        |

Examples:

```sh
./new_ls -A ~
./new_ls -la /etc
./new_ls -R .
./new_ls -St /tmp
./new_ls -- -filename

```

## Design

| Module             | Responsibility                                                            |
| ------------------ | ------------------------------------------------------------------------- |
| `cli_parser.c`     | Phân tích cú pháp `getopt` và xử lý độ ưu tiên của các option             |
| `file_utils.c`     | Nối đường dẫn (path joins) và quản lý danh mục động an toàn bộ nhớ        |
| `scanner.c`        | Đọc thư mục, xử lý các operand, duyệt đệ quy                              |
| `ordering.c`       | Sắp xếp theo filename, size, hoặc timestamp đã chọn                       |
| `formatter.c`      | Long format, quyền hạn (permissions), blocks, mã hóa/thoát ký tự tên file |
| `main.c`           | Điều khiển luồng cấp cao nhất và trả về exit status                       |
| `include/new_ls.h` | Các model và interface dùng chung                                         |

Các system call/API quan trọng: `opendir`, `readdir`, `closedir`, `lstat`, `stat`, `readlink`, `getpwuid`, `getgrgid`, `localtime_r`, `strftime`.

## Validation and limitations

Chạy `make test`, sau đó so sánh với bản cài đặt của **NetBSD** bằng cách sử dụng các đường dẫn giống hệt nhau. Chương trình in một mục trên mỗi dòng, phù hợp với sổ tay thu gọn được cung cấp; bố cục cột tương tác (interactive column layout) cố tình không được cài đặt. Khoảng cách chính xác, các quy ước biến môi trường block-size, quy tắc locale/multibyte, và xử lý whiteout có thể khác biệt so với `ls` chính thức của NetBSD. Bộ smoke-test không thể thay thế cho việc kiểm thử toàn diện trên nhiều nền tảng.

Khi xuất bản nội dung này dưới dạng bài tập môn học, sinh viên nên tự chạy các bài test trong VM NetBSD của riêng mình, hiểu rõ bản cài đặt, và ghi lại các quan sát cũng như đóng góp cá nhân.

```

```
