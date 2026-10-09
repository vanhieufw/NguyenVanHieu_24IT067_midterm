# new_ls

### UNIX File Listing Utility · System Programming Midterm

Trình mô phỏng lệnh `ls` bằng C trên NetBSD

**Sinh viên:** Nguyễn Văn Hiếu – 24IT067  
**Môn học:** Lập trình Hệ thống UNIX – Bài tập giữa kỳ  
**Nền tảng:** NetBSD (có thể biên dịch trên các hệ thống POSIX khác)

---

## Mục lục

| | | |
|---|---|---|
| [Tổng quan](#tổng-quan) | [Tính năng](#tính-năng) | [Công nghệ & môi trường](#công-nghệ--môi-trường) |
| [Cấu trúc dự án](#cấu-trúc-dự-án) | [Kết nối Windows ↔ NetBSD](#kết-nối-windows--netbsd) | [Biên dịch trên NetBSD](#biên-dịch-trên-netbsd) |
| [Cài đặt tùy chọn](#cài-đặt-tùy-chọn-chạy-new_ls-không-cần-) | [Hướng dẫn sử dụng](#hướng-dẫn-sử-dụng) | [Kiểm thử](#kiểm-thử) |
| [Quy trình phát triển](#quy-trình-phát-triển) | [Xử lý sự cố](#xử-lý-sự-cố) | [Kho mã nguồn](#kho-mã-nguồn) |

---

## Tổng quan

**new_ls** là chương trình dòng lệnh mô phỏng một phần chức năng của `ls` trên UNIX, được phát triển **từ đầu bằng ngôn ngữ C**. Dự án tập trung vào việc thao tác trực tiếp với hệ thống tập tin thông qua các API như `opendir()`, `readdir()`, `lstat()`, `stat()` và `readlink()`; **không gọi lệnh `ls` có sẵn** để tạo kết quả.

Mã nguồn được chia thành nhiều mô-đun, có Makefile để biên dịch, bộ kiểm thử tự động và báo cáo dự án. Phạm vi chức năng dựa trên **tài liệu `ls(1)` NetBSD được giảng viên cung cấp**; dự án không nhằm triển khai toàn bộ GNU `ls`.

**Cú pháp:**

```sh
./new_ls [-AacdFfhiklnqRrSstuw] [file ...]
```

- Không truyền đường dẫn: liệt kê thư mục hiện tại.
- Truyền một hoặc nhiều tập tin: in các tập tin tương ứng.
- Truyền thư mục: liệt kê nội dung thư mục, trừ khi dùng `-d`.
- Mặc định: hiển thị một mục trên mỗi dòng.
- Trả mã thoát `0` nếu thành công, khác `0` nếu có lỗi.

> **Trạng thái kiểm thử:** Mã đã được biên dịch và kiểm thử cơ bản. Người sử dụng cần chạy `make` và `make test` trên NetBSD thực tế, đồng thời đối chiếu với `ls` gốc trước khi nộp bài. `make` thành công chỉ xác nhận việc biên dịch, không đảm bảo mọi hành vi đều chính xác.

---

## Tính năng

Mã nguồn bao gồm xử lý cho **19 tùy chọn** trong tài liệu:

| Tùy chọn | Ý nghĩa |
|:--:|---|
| `-a` | Hiển thị tất cả mục, kể cả `.` và `..`. |
| `-A` | Hiển thị mục ẩn nhưng loại trừ `.` và `..`. Tự động bật cho super-user (theo man page). |
| `-c` | Dùng thời điểm thay đổi trạng thái tập tin (ctime) trong chế độ thời gian. |
| `-d` | Hiển thị chính thư mục, không liệt kê nội dung. Ghi đè `-R`. |
| `-F` | Thêm ký hiệu phân loại (`/`, `*`, `@`, `=`, `\|`, `%` …). |
| `-f` | Không sắp xếp danh sách. |
| `-h` | Định dạng kích thước/số block dễ đọc khi đi với `-l` hoặc `-s`. Ghi đè `-k`. |
| `-i` | Hiển thị số inode. |
| `-k` | Dùng đơn vị kilobyte cho thông tin block khi đi với `-s`. Ghi đè `-h`. |
| `-l` | Hiển thị danh sách chi tiết (long format). |
| `-n` | Tương tự `-l`, hiển thị UID/GID dưới dạng số. |
| `-q` | Thay byte không in được bằng `?` (mặc định khi stdout là terminal). |
| `-R` | Duyệt đệ quy các thư mục con. Ghi đè `-d`. |
| `-r` | Đảo ngược thứ tự sắp xếp. |
| `-S` | Sắp xếp theo kích thước giảm dần. |
| `-s` | Hiển thị số block đã sử dụng. |
| `-t` | Sắp xếp theo thời gian, mới nhất trước. |
| `-u` | Dùng thời điểm truy cập (atime) trong chế độ thời gian. |
| `-w` | In nguyên dạng tên tập tin, kể cả ký tự không in được (mặc định khi stdout không phải terminal). |

Các tùy chọn có thể kết hợp, chẳng hạn `-la`, `-ltr`, `-Rla`. Chương trình có xử lý những nhóm tùy chọn mà lựa chọn phía sau ghi đè lựa chọn phía trước, như `-d/-R`, `-l/-n`, `-c/-u`, `-h/-k` và `-q/-w`.

### Hành vi bổ sung theo man page

- Các operand file và directory được tách riêng; file không phải directory được liệt kê trước.
- Symbolic link được đưa vào dòng lệnh sẽ được follow chỉ khi nó trỏ tới directory và không dùng `-d`.
- Long format hiển thị: mode, số link, owner, group, size (hoặc major/minor với device), timestamp, tên file, và `-> target` với symbolic link.
- Số block tuân theo biến môi trường `BLOCKSIZE` (mặc định 512).
- Sticky bit, set-user-ID và set-group-ID được hiển thị đúng (`t`/`T`, `s`/`S`).
- Hỗ trợ whiteout trên NetBSD (kiểu `w` và ký hiệu `%`).
- Exit status: 0 nếu thành công, >0 nếu có lỗi.

---

## Công nghệ & môi trường

| Thành phần | Vai trò |
|---|---|
| **Visual Studio Code (Windows)** | Viết, đọc và chỉnh sửa mã nguồn. |
| **VirtualBox + NetBSD** | Môi trường UNIX để biên dịch và chạy chương trình. |
| **WinSCP (SFTP)** | Chuyển mã nguồn từ Windows sang NetBSD. |
| **`cc`** | Trình biên dịch C trên NetBSD (GCC hoặc Clang, tùy máy). |
| **BSD `make`** | Tự động biên dịch và thực hiện kiểm thử. |
| **POSIX / C11** | API và chuẩn ngôn ngữ dùng trong dự án. |

### Yêu cầu trước khi bắt đầu

1. NetBSD đã được cài và khởi động thành công trong VirtualBox.
2. Có tài khoản người dùng thông thường trên NetBSD và biết mật khẩu.
3. NetBSD có trình biên dịch `cc` và tiện ích `make` (bộ công cụ phát triển).
4. Có WinSCP trên Windows; máy ảo đã bật dịch vụ SSH/SFTP và có kết nối mạng phù hợp.
5. Có VS Code (hoặc editor bất kỳ) để mở dự án. VS Code **không bắt buộc** phải cài trên NetBSD.

Kiểm tra nhanh trong Terminal NetBSD:

```sh
uname -a
command -v cc
command -v make
```

Nếu `cc` không được tìm thấy, cần cài bộ công cụ phát triển tương ứng với bản NetBSD đang dùng (ví dụ bộ **comp**). Nếu `make` không có, kiểm tra lại bản cài đặt NetBSD.

---

## Cấu trúc dự án

```text
new_ls/
├── include/
│   └── new_ls.h           # Khai báo cấu trúc dữ liệu và API chung
├── src/
│   ├── main.c             # Điểm vào chương trình, mã thoát
│   ├── cli_parser.c       # Phân tích tùy chọn dòng lệnh + BLOCKSIZE
│   ├── file_utils.c       # Danh sách động, xử lý đường dẫn, stat
│   ├── scanner.c          # Đọc thư mục, xử lý đối số, đệ quy
│   ├── ordering.c         # Các chế độ sắp xếp
│   └── formatter.c        # Định dạng và in thông tin tập tin
├── tests/
│   └── test.sh            # Bộ kiểm thử tự động
├── Makefile               # Biên dịch / kiểm thử / dọn dẹp / cài đặt
├── .gitignore             # Loại trừ binary và object files
├── .gitattributes         # Giữ định dạng LF khi dùng Git
└── README.md              # Hướng dẫn sử dụng và báo cáo
```

Khi chạy `make`, thư mục dự án sẽ có thêm chương trình thực thi `new_ls` và các file `.o`. Đây là sản phẩm biên dịch, không phải mã nguồn cần sửa trực tiếp.

---

## Kết nối Windows ↔ NetBSD

Có thể bỏ qua mục này nếu **WinSCP của bạn đã kết nối thành công** tới máy ảo NetBSD.

### Cấu hình NAT trong VirtualBox

Tắt máy ảo trước khi điều chỉnh cấu hình. Trong VirtualBox, chọn máy ảo NetBSD → **Settings → Network → Adapter 1**:

1. Bật **Enable Network Adapter**.
2. Đặt **Attached to: NAT**.
3. Mở **Advanced → Port Forwarding**.
4. Thêm quy tắc:

| Trường     | Giá trị ví dụ   |
|------------|-----------------|
| Name       | `SSH`           |
| Protocol   | `TCP`           |
| Host IP    | `127.0.0.1`     |
| Host Port  | `2222`          |
| Guest IP   | Để trống        |
| Guest Port | `22`            |

Cổng `2222` chỉ là ví dụ, có thể thay nếu cổng đã bị sử dụng. Nếu bạn đang dùng **Bridged Adapter** và kết nối trực tiếp qua IP của NetBSD, hãy dùng địa chỉ IP và cổng SSH thực tế thay cho `127.0.0.1:2222`.

### Bật SSH trên NetBSD

Đăng nhập NetBSD và chuyển quyền quản trị:

```sh
su -
```

Mở `/etc/rc.conf`:

```sh
vi /etc/rc.conf
```

Đảm bảo có cấu hình:

```sh
sshd=YES
```

Lưu file và khởi động dịch vụ (nếu chưa chạy):

```sh
/etc/rc.d/sshd start
```

Nếu dịch vụ đã chạy, bạn không cần khởi động lại. Thoát tài khoản root:

```sh
exit
```

**An toàn:** Sử dụng tài khoản người dùng bình thường để truyền file và thực hiện dự án; không cần bật đăng nhập SSH bằng root.

### Kết nối bằng WinSCP

Mở WinSCP → **New Site** và điền:

| Trường        | Ví dụ với NAT Port Forwarding |
|---------------|-------------------------------|
| File protocol | `SFTP`                        |
| Host name     | `127.0.0.1`                   |
| Port number   | `2222`                        |
| User name     | `<ten-tai-khoan-NetBSD>`      |
| Password      | Mật khẩu của tài khoản đó     |

Nhấn **Login**. Lần đầu kết nối, kiểm tra fingerprint máy chủ trước khi chấp nhận. Sau khi đăng nhập, giao diện WinSCP sẽ hiển thị file Windows ở một bên và file NetBSD ở bên còn lại.

### Chuyển dự án vào máy ảo

1. Giải nén file ZIP của dự án trên Windows (hoặc clone từ GitHub).
2. Trong VS Code, mở thư mục `new_ls` để xem hoặc chỉnh sửa code.
3. Mở WinSCP, duyệt tới thư mục home của tài khoản NetBSD (`~`).
4. Kéo **cả thư mục `new_ls`** từ Windows sang NetBSD.
5. Kiểm tra NetBSD có file `~/new_ls/Makefile` và `~/new_ls/src/main.c`.

> **Tránh lỗi thư mục lồng nhau:** Đường dẫn đúng phải là `~/new_ls/Makefile`, không phải `~/new_ls/new_ls/Makefile`. Không nên chuyển riêng từng file `.c` mà bỏ quên `include/`, `Makefile` hay `tests/`.

---

## Biên dịch trên NetBSD

Tại Terminal NetBSD, đăng nhập bằng tài khoản đã nhận source và chạy:

```sh
cd ~/new_ls
make
```

Nếu thành công, chương trình thực thi tên `new_ls` được tạo ở thư mục gốc dự án. Kiểm tra:

```sh
ls -l ./new_ls
```

Chạy thử:

```sh
./new_ls
```

**Vì sao dùng `./new_ls` thay vì `new_ls`?** Dấu `./` yêu cầu shell chạy file thực thi trong thư mục hiện tại; thư mục hiện tại thường không nằm trong biến môi trường `PATH`.

Các lệnh Makefile:

| Lệnh                 | Tác dụng                                                              |
|----------------------|-----------------------------------------------------------------------|
| `make`               | Biên dịch chương trình `new_ls`.                                      |
| `make test`          | Chạy bộ kiểm thử tự động trong `tests/test.sh`.                       |
| `make clean`         | Xóa `new_ls` và các file `.o` đã biên dịch.                           |
| `make clean && make` | Biên dịch sạch lại từ đầu.                                            |
| `make install`       | Biên dịch nếu cần và cài `new_ls` vào `/usr/local/bin` (cần quyền ghi). |
| `make uninstall`     | Gỡ bản `new_ls` đã cài khỏi `/usr/local/bin` (cần quyền ghi).         |

Không cần sử dụng `gcc` riêng lẻ vì Makefile đã quản lý việc biên dịch tất cả các module.

---

## Cài đặt tùy chọn: chạy `new_ls` không cần `./`

> **Không bắt buộc.** Để làm và kiểm thử bài giữa kỳ, chỉ cần `make` rồi chạy `./new_ls`. Phần này dành cho người muốn sử dụng `new_ls` như một lệnh thông thường từ mọi thư mục. Chỉ thực hiện nếu Makefile của phiên bản dự án đã có hai mục tiêu `install` và `uninstall`.

### Cách 1: Cài vào `/usr/local/bin` (cần quyền root)

Đây là cách chuẩn nhất để thầy hoặc người chấm bài có thể chạy `new_ls` từ bất kỳ thư mục nào.

**Bước 1.** Vào thư mục dự án và kiểm thử:

```sh
cd ~/new_ls
make
make test
```

Nếu thấy dòng `PASS: new_ls smoke tests` thì chương trình đã sẵn sàng.

**Bước 2.** Chuyển quyền root (nhập mật khẩu root khi được hỏi):

```sh
su
```

**Bước 3.** Cài đặt (vẫn đang ở thư mục `~/new_ls`):

```sh
make install
```

Lệnh này sao chép chương trình sang `/usr/local/bin/new_ls`. Nó **không thay thế** lệnh `ls` gốc của NetBSD.

**Bước 4.** Thoát root:

```sh
exit
```

**Bước 5.** Kiểm tra kết quả:

```sh
ls -l /usr/local/bin/new_ls
command -v new_ls
new_ls -la
```

Nếu `command -v new_ls` trả về `/usr/local/bin/new_ls` thì có thể chạy trực tiếp từ mọi nơi:

```sh
new_ls
new_ls -la /etc
new_ls -R /tmp
```

### Cách 2: Cài vào thư mục home (không cần quyền root)

Phù hợp khi không có mật khẩu root hoặc không muốn cài toàn hệ thống.

```sh
cd ~/new_ls
make PREFIX=$HOME/.local install
export PATH="$HOME/.local/bin:$PATH"
```

Kiểm tra:

```sh
command -v new_ls
new_ls -la
```

Để PATH có hiệu lực lâu dài (Bash):

```sh
printf '%s\n' 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
. ~/.bashrc
```

### Nếu đã cài nhưng báo `new_ls: command not found`

Nguyên nhân thường là `/usr/local/bin` (hoặc `$HOME/.local/bin`) chưa có trong `PATH`. Kiểm tra:

```sh
echo "$PATH"
```

Với **Bash**, thêm tạm thời cho phiên hiện tại:

```sh
export PATH="/usr/local/bin:$PATH"
```

Muốn áp dụng lâu dài:

```sh
printf '%s\n' 'export PATH="/usr/local/bin:$PATH"' >> ~/.bashrc
. ~/.bashrc
```

Với **csh/tcsh**:

```csh
set path = ( /usr/local/bin $path )
rehash
```

**Không** thêm dấu `./` vào PATH và **không** đổi tên `new_ls` thành `ls`.

### Cập nhật hoặc gỡ cài đặt

Sau khi sửa code, biên dịch và kiểm thử lại, rồi cài lại phiên bản mới:

```sh
cd ~/new_ls
make
make test
su
make install
exit
```

Nếu không muốn dùng lệnh `new_ls` đã cài nữa:

```sh
cd ~/new_ls
su
make uninstall
exit
```

Lệnh gỡ cài đặt chỉ xóa `/usr/local/bin/new_ls`, **không xóa source code** và không ảnh hưởng lệnh `ls` gốc.

---

## Hướng dẫn sử dụng

Các ví dụ dưới đây dùng `./new_ls` để có thể chạy ngay sau khi `make` trong thư mục `~/new_ls`. Nếu đã thực hiện phần **cài đặt tùy chọn** và `PATH` được cấu hình đúng, bạn có thể thay `./new_ls` bằng `new_ls` ở tất cả ví dụ.

### Liệt kê thư mục hiện tại

```sh
./new_ls
```

### Liệt kê một đường dẫn cụ thể

```sh
./new_ls /etc
./new_ls /tmp
./new_ls src/main.c
```

### Hiển thị tập tin ẩn

```sh
./new_ls -a .     # Bao gồm . và ..
./new_ls -A .     # Bỏ qua . và ..
```

### Hiển thị thông tin chi tiết

```sh
./new_ls -l .
./new_ls -la .
./new_ls -n .     # UID/GID dạng số
./new_ls -lh .    # Kích thước dễ đọc
```

Chế độ `-l` hiển thị các trường như loại và quyền tập tin, số liên kết, chủ sở hữu, nhóm, kích thước, thời gian và tên; với symbolic link có thể kèm `->` và đích liên kết.

### Sắp xếp và duyệt đệ quy

```sh
./new_ls -S .       # Kích thước giảm dần
./new_ls -t .       # Thời gian mới nhất trước
./new_ls -tr .      # Thời gian cũ nhất trước
./new_ls -r .       # Đảo thứ tự
./new_ls -R .       # Bao gồm các thư mục con
./new_ls -Rla .     # Đệ quy, long format, hiện file ẩn
```

Cẩn thận với `-R` trên thư mục rất lớn, vì lượng kết quả có thể nhiều.

### In inode, block và phân loại tập tin

```sh
./new_ls -i .
./new_ls -s .
./new_ls -sk .
./new_ls -sh .
./new_ls -F .
./new_ls -lF /etc
```

### Nhiều đường dẫn và tên bắt đầu bằng dấu `-`

```sh
./new_ls /etc /tmp
./new_ls -- -ten-file
```

`--` đánh dấu kết thúc danh sách tùy chọn: các đối số phía sau được hiểu là đường dẫn.

### Kết hợp tùy chọn phổ biến

```sh
./new_ls -la /etc
./new_ls -hls ~
./new_ls -Rla .
./new_ls -St /tmp
./new_ls -F /usr/bin
./new_ls -icu /var
```

---

## Kiểm thử

### Kiểm thử tự động

```sh
cd ~/new_ls
make test
```

Script `tests/test.sh` tạo thư mục thử nghiệm tạm, kiểm tra các tình huống cơ bản (tập tin ẩn, symbolic link, quyền thực thi, một số cách sắp xếp, long format, đệ quy, tùy chọn sai, đường dẫn không tồn tại...) rồi dọn dữ liệu tạm. Khi vượt qua tất cả phép kiểm tra, script in:

```text
PASS: new_ls smoke tests
```

Đây là **smoke test**, không phải bằng chứng rằng mọi tổ hợp của 19 tùy chọn đều đã được kiểm thử đầy đủ. Nếu test thất bại, xem dòng lỗi trong Terminal và kiểm tra lại code, môi trường, khác biệt giữa BSD và GNU utilities.

### So sánh với `ls` của NetBSD

Chương trình này mặc định in **mỗi tập tin một dòng** theo tài liệu được giao. Khi so sánh bản mặc định, lệnh `ls` gốc có thể hiển thị nhiều cột tùy môi trường; để tránh so sánh sai do định dạng terminal, hãy dùng cùng một kiểu đầu ra có thể đối chiếu, chẳng hạn khi ghi ra file:

```sh
ls -1a /tmp > /tmp/ls-original.txt
./new_ls -a /tmp > /tmp/ls-new_ls.txt
diff -u /tmp/ls-original.txt /tmp/ls-new_ls.txt
```

Nếu `diff` không in gì, **hai file đầu ra giống nhau** trong trường hợp đang kiểm tra. Lặp lại phép so sánh với thư mục thử nghiệm nhỏ và các tùy chọn khác. Với `-l`, `-s`, `-h`, cần xem cả quy tắc định dạng và đơn vị block, không chỉ so sánh chuỗi máy móc.

Ví dụ so sánh long format:

```sh
ls -la /tmp > /tmp/ls-l-original.txt
./new_ls -la /tmp > /tmp/ls-l-new_ls.txt
diff -u /tmp/ls-l-original.txt /tmp/ls-l-new_ls.txt
```

### Kiểm thử các trường hợp lỗi

```sh
./new_ls /duong-dan-khong-ton-tai
./new_ls -z
./new_ls /etc /duong-dan-khong-ton-tai
```

Kiểm tra chương trình thông báo lỗi, tiếp tục xử lý các đường dẫn hợp lệ khi có thể, và trả mã thoát khác `0` nếu có lỗi:

```sh
./new_ls /duong-dan-khong-ton-tai
echo $?
```

Ngoài ra nên kiểm thử thư mục rỗng, file tên có khoảng trắng, liên kết tượng trưng bị hỏng và thư mục không có quyền truy cập:

```sh
mkdir empty_dir
./new_ls empty_dir

touch "file with spaces"
./new_ls "file with spaces"

ln -s /khong-ton-tai broken_link
./new_ls -l broken_link
```

---

## Quy trình phát triển

Mỗi lần thay đổi mã nguồn:

1. **Windows / VS Code:** sửa các file `.c`, `.h` và nhấn **Ctrl + S**.
2. **WinSCP:** tải lên NetBSD **những file đã thay đổi**; chọn ghi đè file cũ khi cần.
3. **NetBSD:** chạy `cd ~/new_ls && make`.
4. **NetBSD:** chạy `make test` và kiểm tra thêm lệnh `./new_ls` liên quan tới phần vừa sửa.
5. **Nếu có lỗi:** đọc thông báo trình biên dịch hoặc kết quả test, sửa trên VS Code rồi đồng bộ lại.

**Không chỉnh sửa file `.o` hoặc chương trình `new_ls` bằng tay.** Các file này được tạo tự động từ mã nguồn.

### Quy trình Git khuyến nghị

```sh
git status
git add .
git commit -m "Mô tả ngắn gọn thay đổi"
git push origin main
```

Không commit file nhị phân (`new_ls`), file object (`.o`) hoặc file tạm. File `.gitignore` đã cấu hình sẵn để loại trừ các file này.

---

## Xử lý sự cố

| Hiện tượng | Nguyên nhân có thể | Hướng xử lý |
|---|---|---|
| WinSCP báo `Connection refused` | SSH chưa chạy hoặc NAT forward chưa đúng | Kiểm tra `sshd`, địa chỉ IP và cổng SSH. |
| WinSCP báo `Authentication failed` | Sai username/mật khẩu | Dùng tài khoản NetBSD bình thường, kiểm tra thông tin đăng nhập. |
| `make: not found` | Thiếu công cụ phát triển hoặc PATH | Kiểm tra `command -v make` và bộ cài NetBSD. |
| `cc: not found` | Thiếu trình biên dịch | Kiểm tra `command -v cc`, cài bộ công cụ phát triển phù hợp. |
| `don't know how to make ...` | Sai thư mục hoặc mục tiêu Makefile | Chạy `pwd`, `ls -l Makefile`, kiểm tra tên target. |
| `./new_ls: not found` | Chưa biên dịch hoặc đứng sai thư mục | Vào `~/new_ls`, chạy `make`, kiểm tra `ls -l new_ls`. |
| `Permission denied` khi liệt kê | Không có quyền đọc đường dẫn hoặc chạy file | Kiểm tra quyền tập tin bằng `ls -l`; không tự ý dùng root để đọc dữ liệu. |
| `make install` báo `mkdir: Permission denied` | Không có quyền ghi vào `/usr/local/bin` | Chuyển sang root bằng `su`, rồi chạy `make install` trong thư mục dự án. |
| `new_ls: command not found` sau khi cài | `/usr/local/bin` không có trong `PATH` | Kiểm tra `command -v new_ls`; thêm `/usr/local/bin` vào `PATH` như hướng dẫn ở trên. |
| `make test` thất bại | Lỗi hành vi hoặc khác biệt công cụ môi trường | Xem lỗi đầu tiên, đối chiếu bằng test đơn lẻ và `ls` gốc. |
| Sửa code mà kết quả không đổi | Chưa tải file mới lên máy ảo | Kiểm tra WinSCP đã ghi đè đúng file trong `~/new_ls/src/`. |
| Lỗi `undefined reference` khi link | Thiếu file `.c` trong Makefile | Kiểm tra danh sách `SOURCES` trong Makefile. |
| Tên tháng hiện sai / ký tự lạ | Locale chưa được thiết lập | Chương trình đã gọi `setlocale(LC_ALL, "")`; kiểm tra biến `LANG` trên hệ thống. |

---

## Kho mã nguồn

**GitHub:** https://github.com/vanhieufw/NguyenVanHieu_24IT067_midterm

### Quy trình đẩy code lên GitHub

```sh
# Lần đầu (nếu chưa clone)
git clone https://github.com/vanhieufw/NguyenVanHieu_24IT067_midterm.git
cd NguyenVanHieu_24IT067_midterm

# Mỗi lần thay đổi
git status
git add .
git commit -m "Mô tả thay đổi: ví dụ Fix long format alignment"
git push origin main
```

**Lưu ý quan trọng khi nộp bài:**

- Repository phải chứa: `Makefile`, `README.md`, toàn bộ source (`.c`, `.h`), `.gitignore`.
- **Không** commit compiled binaries, object files (`.o`), hoặc file hệ thống không cần thiết.
- Đưa link GitHub vào báo cáo và nộp qua hệ thống e-learning theo hạn của giảng viên.

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

---

**new_ls · UNIX System Programming Midterm**  
Written in C · Built with BSD Make · Runs on NetBSD
