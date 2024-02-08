Bootloader for STM32F411

The application does:
1) Read a HEX file. For example we use 5 lines with various data.
2) Check the CRC code validity of each line and analise the HEX file structure.
3) Write the data to Flash.
4) Output the data via UART.

The protection circuit is also implemented in hardware.
