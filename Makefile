# Trình biên dịch(GNU C Compiler)
CC = gcc

# Các cờ (flags) cho trình biên dịch:
# -Wall: Bật tất cả các cảnh báo
# -g: Thêm thông tin gỡ lỗi (cho GDB)
# -o: Chỉ định tên file đầu ra
CFLAGS = -Wall -g

# File thực thi
EXECUTABLE = RC4

# File mã nguồn (.c)
SOURCES = RC4.c RC4_encrypt_main.c read_test.c user.c 

# Quy tắc mặc định: Chạy khi gõ "make"
# Phụ thuộc vào file thực thi
all: $(EXECUTABLE)

# Quy tắc chính: Cách để tạo ra file thực thi
# Phụ thuộc vào tất cả các file mã nguồn
$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

# Quy tắc "clean": Dọn dẹp dự án
# Xóa file thực thi đã tạo
clean:
	rm -f $(EXECUTABLE)
