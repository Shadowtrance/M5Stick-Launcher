esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_cplus1_1.bin      0x1000 .pio/build/m5stack-cplus1_1/bootloader.bin   0x8000 .pio/build/m5stack-cplus1_1/partitions.bin                                                                             0x10000 .pio/build/m5stack-cplus1_1/firmware.bin
esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_C.bin             0x1000 .pio/build/m5stack-c/bootloader.bin          0x8000 .pio/build/m5stack-c/partitions.bin                                                                                    0x10000 .pio/build/m5stack-c/firmware.bin
esptool --chip esp32s3 merge_bin --output Launcher/Launcher_2.0_Cardputer.bin   0x0 .pio/build/m5stack-cardputer/bootloader.bin     0x8000 .pio/build/m5stack-cardputer/partitions.bin  0x9000 support_files/UiFlow2_nvs.bin                                      0x10000 .pio/build/m5stack-cardputer/firmware.bin
esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_cplus2.bin        0x1000 .pio/build/m5stack-cplus2/bootloader.bin     0x8000 .pio/build/m5stack-cplus2/partitions.bin     0x9000 support_files/UiFlow2_nvs.bin                                      0x10000 .pio/build/m5stack-cplus2/firmware.bin
esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_core2.bin         0x1000 .pio/build/m5stack-core2/bootloader.bin      0x8000 .pio/build/m5stack-core2/partitions.bin      0x9000 support_files/UiFlow2_nvs.bin 0xF000 support_files/UiFlow1_phi.bin 0x10000 .pio/build/m5stack-core2/firmware.bin
esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_core_16Mb.bin     0x1000 .pio/build/m5stack-core/bootloader.bin       0x8000 .pio/build/m5stack-core/partitions.bin       0x9000 support_files/UiFlow2_nvs.bin 0xF000 support_files/UiFlow1_phi.bin 0x10000 .pio/build/m5stack-core/firmware.bin
esptool --chip esp32s3 merge_bin --output Launcher/Launcher_2.0_CoreS3.bin      0x0 .pio/build/m5stack-cores3/bootloader.bin        0x8000 .pio/build/m5stack-cores3/partitions.bin     0x9000 support_files/UiFlow2_nvs.bin 0xF000 support_files/UiFlow1_phi.bin 0x10000 .pio/build/m5stack-cores3/firmware.bin

esptool --chip esp32s3 merge_bin --output Launcher/Launcher_2.0_T-Display-S3-Touchscreen.bin        0x0 .pio/build/lilygo-t-display-S3/bootloader.bin           0x8000 .pio/build/lilygo-t-display-S3/partitions.bin                                                        0x10000 .pio/build/lilygo-t-display-S3/firmware.bin
esptool --chip esp32s3 merge_bin --output Launcher/Launcher_2.0_T-Embed.bin                         0x0 .pio/build/lilygo-t-embed/bootloader.bin                0x8000 .pio/build/lilygo-t-embed/partitions.bin                                                             0x10000 .pio/build/lilygo-t-embed/firmware.bin
esptool --chip esp32s3 merge_bin --output Launcher/Launcher_2.0_T-Embed_CC1101.bin                  0x0 .pio/build/lilygo-t-embed-cc1101/bootloader.bin         0x8000 .pio/build/lilygo-t-embed-cc1101/partitions.bin                                                      0x10000 .pio/build/lilygo-t-embed-cc1101/firmware.bin
esptool --chip esp32s3 merge_bin --output Launcher/Launcher_2.0_T-Deck.bin                          0x0 .pio/build/lilygo-t-deck/bootloader.bin                 0x8000 .pio/build/lilygo-t-deck/partitions.bin                                                              0x10000 .pio/build/lilygo-t-deck/firmware.bin

esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_CYD-2432S028.bin  0x1000 .pio/build/CYD-2432S028/bootloader.bin        0x8000 .pio/build/CYD-2432S028/partitions.bin                                                                                0x10000 .pio/build/CYD-2432S028/firmware.bin
esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_CYD-2-USB.bin     0x1000 .pio/build/CYD-2-USB/bootloader.bin           0x8000 .pio/build/CYD-2-USB/partitions.bin                                                                                   0x10000 .pio/build/CYD-2-USB/firmware.bin

esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_Marauder-v4-OG.bin 0x1000 .pio/build/Marauder-v4-OG/bootloader.bin     0x8000 .pio/build/Marauder-v4-OG/partitions.bin   0x10000 .pio/build/Marauder-v4-OG/firmware.bin
esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_Marauder-Mini.bin 0x1000 .pio/build/Marauder-Mini/bootloader.bin       0x8000 .pio/build/Marauder-Mini/partitions.bin    0x10000 .pio/build/Marauder-Mini/firmware.bin

esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_headless-esp32-4mb.bin 0x1000 .pio/build/headless-esp32-4mb/bootloader.bin       0x8000 .pio/build/headless-esp32-4mb/partitions.bin    0x10000 .pio/build/headless-esp32-4mb/firmware.bin
esptool --chip esp32 merge_bin --output Launcher/Launcher_2.0_headless-esp32-8mb.bin 0x1000 .pio/build/headless-esp32-8mb/bootloader.bin       0x8000 .pio/build/headless-esp32-8mb/partitions.bin    0x10000 .pio/build/headless-esp32-8mb/firmware.bin

esptool --chip esp32s3 merge_bin --output Launcher/Launcher_2.0_headless-esp32s3-8mb.bin          0x0 .pio/build/headless-esp32s3-8mb/bootloader.bin                 0x8000 .pio/build/headless-esp32s3-8mb/partitions.bin                                                              0x10000 .pio/build/headless-esp32s3-8mb/firmware.bin
esptool --chip esp32s3 merge_bin --output Launcher/Launcher_2.0_headless-esp32s3-16mb.bin          0x0 .pio/build/headless-esp32s3-16mb/bootloader.bin                 0x8000 .pio/build/headless-esp32s3-16mb/partitions.bin                                                              0x10000 .pio/build/headless-esp32s3-16mb/firmware.bin