# Bootloader-for-STM32
Parsing a HEX-file and loading data into flash memory of STM32
Программное обеспечение выполняет следующие действия:
Чтение строк HEX файла. Для примера используется 5 строк с разнообразными данными.
Программа проверяет корректность CRC кода считанной строки, анализирует структуру записи HEX файла и помещает данные в память микроконтроллера.
