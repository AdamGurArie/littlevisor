SRC_DIR = src
BUILD_DIR = build
LINKER_FILE = linker.ld
OUTPUT := $(BUILD_DIR)/kernel.elf
ISO := $(BUILD_DIR)/kernel.iso
COMPILATION_FLAGS := -g -Wall -Wextra -pipe -fno-stack-protector -mno-red-zone -fpermissive -nostdlib -std=c++20 $(INCLUDE_PATH)

LDFLAGS ?= -nostd $(INCLUDE_PATH)

override INTERNALCFLAGS :=   \
	-I.                  \
	-ffreestanding       \
	-fno-stack-protector \
	-fno-pic             \
	-mabi=sysv           \
	-mno-80387           \
	-mno-mmx             \
	-mno-3dnow           \
	-mno-sse             \
	-mno-sse2            \
	-mno-red-zone        \
	-mcmodel=kernel      \
	-fsanitize=address   \
	-static-libasan      \
	-MMD

override INTERNALLDFLAGS :=    \
	-Tlinker.ld            \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static


SRC_FILES = $(shell find $(SRC_DIR) -name '*.cpp')
OBJECT_FILES = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SRC_FILES:.cpp=.o))

build-iso: $(ISO)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	g++ $(COMPILATION_FLAGS) -c $< -o $@

$(OUTPUT): $(OBJECT_FILES)
	@mkdir -p $(BUILD_DIR)
	@echo $(OBJECT_FILES)
	ld $(LDFLAGS) $(INTERNALLDFLAGS) -o $(OUTPUT) $(OBJECT_FILES)

$(ISO): $(OUTPUT) limine
	mkdir -p iso_root/boot 
	cp -v build/kernel.elf iso_root/boot/
	mkdir -p iso_root/boot/limine
	cp -v limine.cfg limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	mkdir -p iso_root/EFI/BOOT 
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(ISO)
	./limine/limine bios-install $(ISO)
	qemu-system-x86_64 -debugcon stdio -cdrom $(ISO) -boot d -no-reboot -no-shutdown

debug:
	qemu-system-x86_64 -cdrom $(ISO) -debugcon stdio -boot d -no-reboot -no-shutdown -s -S

clean:
	@rm -rf $(BUILD_DIR)
