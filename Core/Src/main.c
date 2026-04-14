/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "quadspi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define WRITE_ENABLE_CMD   0x06U
#define READ_STATUS_REG1_CMD   0x05U
#define READ_STATUS_REG2_CMD   0x35U
#define READ_STATUS_REG3_CMD   0x15U
#define WRITE_STATUS_REG2_CMD 	0x31U
#define QSPI_TIMEOUT_DEFAULT_VALUE 1000U
#define QUAD_OUT_FAST_READ_CMD       0x6BU
#define DUMMY_CLOCK_CYCLES_READ_QUAD 8U
#define QUAD_IN_FAST_PROG_CMD 0x32U
#define MEMORY_PAGE_SIZE 		256U
#define SECTOR_ERASE_CMD        0x20U
#define CHIP_ERASE_CMD          0xC7U
#define MEMORY_SECTOR_SIZE      0x1000U   /* 4KB */
#define QUADSPI_MAX_ERASE_TIMEOUT 300000U
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t QSPI_ReadJedecID(void);
static uint8_t QSPI_ResetChip(void)
{
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode = QSPI_ADDRESS_NONE;
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode = QSPI_DATA_NONE;
    sCommand.NbData = 0U;
    sCommand.DummyCycles = 0U;
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Instruction = 0x66; // Reset Enable
    if (HAL_QSPI_Command(&hqspi, &sCommand, 1000) != HAL_OK)
    {
    	return HAL_ERROR;
    }

    sCommand.Instruction = 0x99; // Reset
    if (HAL_QSPI_Command(&hqspi, &sCommand, 1000) != HAL_OK)
    {
    	return HAL_ERROR;
    }

    HAL_Delay(1);

    return HAL_OK;
}

static uint8_t QSPI_WriteEnable(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    QSPI_AutoPollingTypeDef sConfig = {0};

    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Send Write Enable command (0x06) */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = WRITE_ENABLE_CMD;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0U;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Wait until WEL bit is set (SR1 bit1) */
    sCommand.Instruction = READ_STATUS_REG1_CMD;  /* 0x05 */
    sCommand.DataMode    = QSPI_DATA_1_LINE;
    sCommand.NbData      = 1U;

    sConfig.Match           = 0x02U;   /* WEL = 1 */
    sConfig.Mask            = 0x02U;
    sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1U;
    sConfig.Interval        = 0x10;
    sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static uint8_t QSPI_ReadStatusReg1(uint8_t *value)
{
    QSPI_CommandTypeDef sCommand = {0};

    if (value == NULL)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_STATUS_REG1_CMD;   /* 0x05 */
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0U;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.NbData            = 1U;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, value, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static uint8_t QSPI_ReadStatusReg2(uint8_t *value)
{
    QSPI_CommandTypeDef sCommand = {0};

    if (value == NULL)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_STATUS_REG2_CMD;   /* 0x35 */
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0U;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.NbData            = 1U;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, value, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static uint8_t QSPI_ReadStatusReg3(uint8_t *value)
{
    QSPI_CommandTypeDef sCommand = {0};

    if (value == NULL)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_STATUS_REG3_CMD;   /* 0x15 */
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0U;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.NbData            = 1U;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, value, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static uint8_t QSPI_AutoPollingMemReady(uint32_t Timeout)
{
    QSPI_CommandTypeDef sCommand = {0};
    QSPI_AutoPollingTypeDef sConfig = {0};

    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Read Status Register-1 (0x05) */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = READ_STATUS_REG1_CMD; /* 0x05 */
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0U;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.NbData            = 1U;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Wait until BUSY = 0 */
    sConfig.Match           = 0x00U;
    sConfig.Mask            = 0x01U;  /* BUSY bit */
    sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize = 1U;
    sConfig.Interval        = 0x10;
    sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, Timeout) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint8_t QSPI_WaitBusy(uint32_t Timeout)
{
    uint8_t sr1 = 0;
    uint32_t tickstart = HAL_GetTick();

    do
    {
        if (QSPI_ReadStatusReg1(&sr1) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if ((sr1 & 0x01U) == 0U)   /* BUSY = 0 */
        {
            return HAL_OK;
        }

    } while ((HAL_GetTick() - tickstart) < Timeout);

    return HAL_TIMEOUT;
}

static uint8_t QSPI_WriteStatusReg2(uint8_t value)
{
    QSPI_CommandTypeDef sCommand = {0};

    if (QSPI_WriteEnable() != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = WRITE_STATUS_REG2_CMD;   /* 0x31 */
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0U;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.NbData            = 1U;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Transmit(&hqspi, &value, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (QSPI_AutoPollingMemReady(QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

static uint8_t QSPI_EnableQuadMode(void)
{
    uint8_t sr2 = 0U;

    if (QSPI_ReadStatusReg2(&sr2) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if ((sr2 & 0x02U) == 0U)
    {
        sr2 |= 0x02U;

        if (QSPI_WriteStatusReg2(sr2) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (QSPI_ReadStatusReg2(&sr2) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if ((sr2 & 0x02U) == 0U)
        {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

uint8_t CSP_QSPI_ReadMemory(uint8_t *buffer, uint32_t address, uint32_t size)
{
    QSPI_CommandTypeDef sCommand = {0};

    if ((buffer == NULL) || (size == 0U))
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = QUAD_OUT_FAST_READ_CMD;   /* 0x6B */
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = address;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = size;
    sCommand.DummyCycles       = DUMMY_CLOCK_CYCLES_READ_QUAD; /* 8 */
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, buffer, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint8_t CSP_QSPI_WriteMemory(uint8_t *buffer, uint32_t address, uint32_t buffer_size)
{
    QSPI_CommandTypeDef sCommand = {0};
    uint32_t current_addr;
    uint32_t current_size;
    uint32_t end_addr;

    if ((buffer == NULL) || (buffer_size == 0U))
    {
        return HAL_ERROR;
    }

    current_addr = address;
    current_size = MEMORY_PAGE_SIZE - (address % MEMORY_PAGE_SIZE);
    if (current_size > buffer_size)
    {
        current_size = buffer_size;
    }

    end_addr = address + buffer_size;

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = QUAD_IN_FAST_PROG_CMD;   /* 0x32 */
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;

    do
    {
        sCommand.Address = current_addr;
        sCommand.NbData  = current_size;

        if (QSPI_WriteEnable() != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (HAL_QSPI_Transmit(&hqspi, buffer, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (QSPI_WaitBusy(QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return HAL_ERROR;
        }

        current_addr += current_size;
        buffer       += current_size;

        if (current_addr >= end_addr)
        {
            break;
        }

        current_size = ((current_addr + MEMORY_PAGE_SIZE) > end_addr)
                     ? (end_addr - current_addr)
                     : MEMORY_PAGE_SIZE;

    } while (current_addr < end_addr);

    return HAL_OK;
}

uint8_t CSP_QSPI_EraseSector(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
    QSPI_CommandTypeDef sCommand = {0};

    EraseStartAddress = EraseStartAddress - (EraseStartAddress % MEMORY_SECTOR_SIZE);

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = SECTOR_ERASE_CMD;      /* 0x20 */
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.NbData            = 0U;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    while (EraseStartAddress <= EraseEndAddress)
    {
        sCommand.Address = EraseStartAddress;

        if (QSPI_WriteEnable() != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (QSPI_WaitBusy(QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        {
            return HAL_ERROR;
        }

        EraseStartAddress += MEMORY_SECTOR_SIZE;
    }

    return HAL_OK;
}

uint8_t CSP_QSPI_Erase_Chip(void)
{
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = CHIP_ERASE_CMD;        /* 0xC7 */
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0U;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.NbData            = 0U;
    sCommand.DummyCycles       = 0U;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (QSPI_WriteEnable() != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Command(&hqspi, &sCommand, QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (QSPI_WaitBusy(QUADSPI_MAX_ERASE_TIMEOUT) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint8_t CSP_QSPI_EnableMemoryMappedMode(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    QSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = QUAD_OUT_FAST_READ_CMD;   /* 0x6B */
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = 0U;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = 0U;
    sCommand.DummyCycles       = DUMMY_CLOCK_CYCLES_READ_QUAD; /* 8 */
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    sMemMappedCfg.TimeOutPeriod     = 0U;

    if (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_QUADSPI_Init();
  /* USER CODE BEGIN 2 */
  HAL_QSPI_Abort(&hqspi);
  uint8_t status = QSPI_ResetChip();
  HAL_Delay(100);
  uint32_t jedec_id = QSPI_ReadJedecID();

  if (QSPI_EnableQuadMode() != HAL_OK)
  {
      return HAL_ERROR;
  }

  uint8_t sr1 = 0, sr2 = 0, sr3 = 0;

  if (QSPI_ReadStatusReg1(&sr1) != HAL_OK)
  {
      return HAL_ERROR;
  }

  if (QSPI_ReadStatusReg2(&sr2) != HAL_OK)
  {
      return HAL_ERROR;
  }

  if (QSPI_ReadStatusReg3(&sr3) != HAL_OK)
  {
      return HAL_ERROR;
  }

  uint8_t tx[16] = {
      0x11, 0x22, 0x33, 0x44,
      0x55, 0x66, 0x77, 0x88,
      0x99, 0xAA, 0xBB, 0xCC,
      0xDD, 0xEE, 0x12, 0x34
  };

  uint8_t rx[16] = {0};

  if (CSP_QSPI_WriteMemory(tx, 0x00000000, sizeof(tx)) != HAL_OK)
  {
      return HAL_ERROR;
  }

  if (CSP_QSPI_ReadMemory(rx, 0x00000000, sizeof(rx)) != HAL_OK)
  {
      return HAL_ERROR;
  }

  /*
  if (CSP_QSPI_EraseSector(0x00000000, 0x00000000) != HAL_OK)
  {
      return HAL_ERROR;
  }

  uint8_t rx2[16] = {0};

  if (CSP_QSPI_ReadMemory(rx2, 0x00000000, sizeof(rx)) != HAL_OK)
  {
      return HAL_ERROR;
  }
  */

  if (CSP_QSPI_EnableMemoryMappedMode() != HAL_OK)
  {
      return HAL_ERROR;
  }

  volatile uint8_t *qspi_mem = (volatile uint8_t *)0x90000000;
  uint8_t a0 = qspi_mem[0];
  uint8_t a1 = qspi_mem[1];
  uint8_t a2 = qspi_mem[2];
  uint8_t a3 = qspi_mem[3];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint32_t QSPI_ReadJedecID(void)
{
	QSPI_CommandTypeDef sCommand = {0};
	uint8_t id[3] = {0};

	HAL_QSPI_Abort(&hqspi);

	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction       = 0x9F; /* JEDEC ID */
	sCommand.AddressMode       = QSPI_ADDRESS_NONE;
	sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
	sCommand.Address 		   = 0U;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode          = QSPI_DATA_1_LINE;
	sCommand.NbData            = 3U;
	sCommand.DummyCycles       = 0U;
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	HAL_StatusTypeDef status;
	status = HAL_QSPI_Command(&hqspi, &sCommand, 1000);
	if (status != HAL_OK)
	{
		return 0;
	}

	status = HAL_QSPI_Receive(&hqspi, id, 1000);
	if (status != HAL_OK)
	{
		return 0;
	}

	return ((uint32_t)id[0] << 16) |
	           ((uint32_t)id[1] << 8)  |
	           ((uint32_t)id[2]);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
