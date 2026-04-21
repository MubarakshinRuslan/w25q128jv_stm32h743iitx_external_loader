# STM32H7 CORE Board External Loader для W25Q128JV (QSPI)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## 📌 Назначение

Этот репозиторий содержит готовый внешний загрузчик (External Loader) для платы **STM32H7 CORE Board** (MCU: `STM32H743IIT6`).

Загрузчик позволяет прошивать внешнюю QSPI Flash память **Winbond W25Q128JV** (128 Мбит) напрямую через утилиту **STM32CubeProgrammer** без написания дополнительных скриптов. Это необходимо для проектов, использующих графику (TouchGFX) или большие массивы данных во внешней памяти.

## 🎯 Решённая задача

Стандартная поставка STM32CubeProgrammer не содержит готового `.stldr` файла для связки `STM32H743` + `W25Q128JV`. Без этого загрузчика программирование QSPI флеш-памяти через ST-Link невозможно — требовалось бы писать отдельную прошивку-прошивальщик.

**Данный проект решает эту проблему и позволяет работать с внешней памятью "из коробки".**

## 🔧 Выполненная работа (Адаптация под H7)

1. **Портирование драйвера QSPI:** За основу взята проверенная реализация интерфейса QSPI из проекта [STM32L431-W25Q128-QSPI-PROJECT](https://github.com/alixahedi/STM32L431-W25Q128-QSPI-PROJECT) (Ali Zahedi-Mobini, лицензия MIT).
2. **Адаптация под архитектуру Cortex-M7 (STM32H7):**
   - Замена HAL-драйверов на актуальные для семейства H7.
   - Корректная настройка **MPU (Memory Protection Unit)** для адресного пространства `0x90000000` (Memory Mapped режим QSPI) с отключением кэширования данных.
   - Настройка тактирования QSPI с учётом частоты ядра 480 МГц для стабильной работы флеш-памяти.
   - Отладка временных параметров стирания секторов (Erase Suspend/Resume).
3. **Сборка:** Генерация финального файла `h743_flash.stldr` и проверка его работоспособности в STM32CubeProgrammer на реальной плате.

## 📟 Аппаратная конфигурация

| Компонент               | Модель / Значение                     |
| :---------------------- | :------------------------------------ |
| **Отладочная плата**    | STM32H7 CORE Board (STM32H743IIT6)    |
| **Внешняя Flash память** | Winbond W25Q128JV (128 Мбит / 16 МБ)  |
| **Интерфейс**           | Quad SPI (Bank 1, Memory Mapped)      |
| **Тактирование QSPI**   | ~80 МГц (Prescaler = 2 от 200 МГц)    |

## 🚀 Инструкция по использованию

### 1. Установка загрузчика в STM32CubeProgrammer

1. Скачайте файл `h743_flash.stldr` из этого репозитория.
2. Скопируйте его в папку установки STM32CubeProgrammer:
```
C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\ExternalLoader
```
3. Перезапустите STM32CubeProgrammer.

### 2. Прошивка внешней Flash памяти

1. Подключите плату к ПК через ST-Link.
2. В STM32CubeProgrammer нажмите **Connect**.
3. В разделе **"External Loader"** найдите в списке `h743_flash` и выберите его.
4. Перейдите на вкладку **"Erasing & Programming"**.
5. Укажите путь к вашему бинарному файлу (например, `TouchGFX.bin`) и **Start Address: 0x90000000**.
6. Нажмите **"Start Programming"**.

### 3. Самостоятельная сборка из исходников (опционально)

1. Откройте проект в **STM32CubeIDE** (файл `.project` в корне).
2. При необходимости измените размер памяти в `quadspi.h` (`#define MEMORY_FLASH_SIZE`).
3. Выполните **Build Project**.
4. Сгенерированный `.stldr` файл появится в папке `Debug` или `Release`.

## 📁 Структура репозитория
├── Core/ # Исходники загрузчика (Init, Erase, Write, Read)
├── Drivers/ # CMSIS и HAL драйверы STM32H7
├── .settings/ # Настройки среды
├── h743_flash.ioc # Файл проекта CubeMX
├── linker.ld # Скрипт линковки для External Loader
├── h743_flash.stldr # ✅ Готовый загрузчик (можно использовать сразу)
└── README.md

## 📜 Лицензия и авторство

**Лицензия:** MIT License (см. файл LICENSE в корне репозитория).

**Основа кода:** Данный проект является адаптацией (портом) открытой разработки **"STM32L431-W25Q128-QSPI-PROJECT"**, созданной [Ali Zahedi-Mobini](https://github.com/alixahedi) и распространяемой под лицензией MIT.

Оригинальный репозиторий: [https://github.com/alixahedi/STM32L431-W25Q128-QSPI-PROJECT](https://github.com/alixahedi/STM32L431-W25Q128-QSPI-PROJECT)

**Автор адаптации для STM32H7:** [Руслан Мубаракшин]
