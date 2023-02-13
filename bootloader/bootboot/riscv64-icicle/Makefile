TARGET=../dist/bootboot.rv64

all: $(TARGET)

$(TARGET):
	@dd if=/dev/zero of=$(TARGET) bs=8192 count=1 2>&1 >/dev/null || true

clean:
	rm -f $(TARGET)

